#include <stdarg.h>
#include <string.h>
#include <ruby.h>
#ifdef __CYGWIN__
/* boolean is defined as a macro in oratypes.h.
 * It conflicts with the definition in windows.h.
 */
#undef boolean
#endif
#include <ociap.h>

#ifdef _WIN32
#define PLRUBY_EXPORT __declspec(dllexport)
#else
#define PLRUBY_EXPORT
#endif

#define MAX_ARG 20

typedef struct plruby_context {
    OCIEnv *envhp;
    OCISvcCtx *svchp;
    OCIError *errhp;
    const char *obj;
    const char *meth;
    int rettype;
    int narg;
    OCIAnyData *args[MAX_ARG];
} plruby_context_t;

static OCIAnyData *call_ruby(plruby_context_t *ctx);
static const char *get_error_msg(int *errnum);
static VALUE to_ruby_value(plruby_context_t *ctx, OCIAnyData *sdata);
static OCIAnyData *to_return_value(plruby_context_t *ctx, VALUE val);

#ifdef PLRUBY_DEBUG
static void debug_msg(const char *fmt, ...);
static OCISvcCtx *dbg_svchp;
static OCIError *dbg_errhp;
#define DEBUG_LOG(msg) output_msg msg
#else
#define DEBUG_LOG(msg) do {} while(0)
#endif


PLRUBY_EXPORT void
Init_extproc_oracle(void)
{
    rb_raise(rb_eRuntimeError, "Don't require 'extproc_ruby' from ruby.");
}

PLRUBY_EXPORT OCIAnyData *
plruby_impl(OCIExtProcContext *with_context, short *ret_ind, int rettype,
            const char *obj, short obj_ind, const char *meth, short meth_ind, int narg, short narg_ind,
            OCIAnyData *a1, short a1_ind, OCIAnyData *a2, short a2_ind, OCIAnyData *a3, short a3_ind,
            OCIAnyData *a4, short a4_ind, OCIAnyData *a5, short a5_ind, OCIAnyData *a6, short a6_ind,
            OCIAnyData *a7, short a7_ind, OCIAnyData *a8, short a8_ind, OCIAnyData *a9, short a9_ind,
            OCIAnyData *a10, short a10_ind, OCIAnyData *a11, short a11_ind, OCIAnyData *a12, short a12_ind,
            OCIAnyData *a13, short a13_ind, OCIAnyData *a14, short a14_ind, OCIAnyData *a15, short a15_ind,
            OCIAnyData *a16, short a16_ind, OCIAnyData *a17, short a17_ind, OCIAnyData *a18, short a18_ind,
            OCIAnyData *a19, short a19_ind, OCIAnyData *a20, short a20_ind)
{
    plruby_context_t ctx;
    OCIAnyData *sdata;
    static int initialized = 0;

    OCIExtProcGetEnv(with_context, &ctx.envhp, &ctx.svchp, &ctx.errhp);
#ifdef PLRUBY_DEBUG
    dbg_svchp = ctx.svchp;
    dbg_errhp = ctx.errhp;
#endif
    ctx.obj = obj_ind ? NULL : obj;
    ctx.meth = meth_ind ? NULL : meth;
    ctx.rettype = rettype;
    ctx.narg = narg_ind ? 0 : narg;
    ctx.args[0] = a1;
    ctx.args[1] = a2;
    ctx.args[2] = a3;
    ctx.args[3] = a4;
    ctx.args[4] = a5;
    ctx.args[5] = a6;
    ctx.args[6] = a7;
    ctx.args[7] = a8;
    ctx.args[8] = a9;
    ctx.args[9] = a10;
    ctx.args[10] = a11;
    ctx.args[11] = a12;
    ctx.args[12] = a13;
    ctx.args[13] = a14;
    ctx.args[14] = a15;
    ctx.args[15] = a16;
    ctx.args[16] = a17;
    ctx.args[17] = a18;
    ctx.args[18] = a19;
    ctx.args[19] = a20;

    if (!initialized) {
        ruby_init();
        ruby_init_loadpath();
        ruby_script("extproc_ruby");
        initialized = 1;
    }

    {
        int state;
        RUBY_INIT_STACK;

        sdata = (OCIAnyData*)rb_protect((VALUE(*)(VALUE))call_ruby, (VALUE)&ctx, &state);
        if (state) {
            int errnum = 20999; /* The last value of user-defined error number */
            const char *errmsg = (const char *)rb_protect((VALUE(*)(VALUE))get_error_msg, (VALUE)&errnum, &state);
            if (state) {
                errmsg = "Failed to get an error in extproc_ruby";
            }
            OCIExtProcRaiseExcpWithMsg(with_context, errnum, (OraText*)errmsg, 0);
        }
    }
    *ret_ind = (sdata != NULL) ? OCI_IND_NOTNULL : OCI_IND_NULL;
    return sdata;
}

static OCIAnyData *
call_ruby(plruby_context_t *ctx)
{
    VALUE obj;

    obj = rb_eval_string(ctx->obj);
    if (ctx->meth != NULL) {
        VALUE args[MAX_ARG];
        int i;

        for (i = 0; i < ctx->narg; i++) {
            args[i] = to_ruby_value(ctx, ctx->args[i]);
        }
        obj = rb_funcall2(obj, rb_intern(ctx->meth), ctx->narg, args);
    }
    return to_return_value(ctx, obj);
}

static const char *
get_error_msg(int *errnum)
{
    VALUE exc = rb_String(rb_errinfo());
    const char *errmsg = StringValueCStr(exc);
    VALUE num = rb_iv_get(exc, "@plsql_error_number");

    if (errmsg == NULL || errmsg[0] == '\0') {
        errmsg = "Unknown extproc_ruby error";
    }
    if (FIXNUM_P(num)) {
        long n = FIX2INT(num);
        if (1 <= n && n <= 32767) {
            *errnum = (int)n;
        }
    }
    return errmsg;
}

static VALUE
to_ruby_value(plruby_context_t *ctx, OCIAnyData *sdata)
{
    boolean isnull;
    OCITypeCode tc;
    OCIType *type;
    OCIInd ind = 0;
    ub4 len;

    OCIAnyDataIsNull(ctx->svchp, ctx->errhp, sdata, &isnull);
    if (isnull) {
        return Qnil;
    }

    OCIAnyDataGetType(ctx->svchp, ctx->errhp, sdata, &tc, &type);
    switch (tc) {
    case OCI_TYPECODE_VARCHAR2:
        {
            OCIString *str;

            OCIAnyDataAccess(ctx->svchp, ctx->errhp, sdata, OCI_TYPECODE_VARCHAR2, NULL, &ind, &str, &len);
            return rb_str_new((char*)OCIStringPtr(ctx->envhp, str), OCIStringSize(ctx->envhp, str));
        }
    case OCI_TYPECODE_NUMBER:
        {
            OCINumber num;
            OCINumber *num_ptr = &num;
            double dval;

            OCIAnyDataAccess(ctx->svchp, ctx->errhp, sdata, OCI_TYPECODE_NUMBER, NULL, &ind, &num_ptr, &len);
            OCINumberToReal(ctx->errhp, num_ptr, sizeof(dval), &dval);
            return rb_float_new(dval);
        }
    case OCI_TYPECODE_BDOUBLE:
        {
            double dval;
            double *dval_ptr = &dval;

            OCIAnyDataAccess(ctx->svchp, ctx->errhp, sdata, OCI_TYPECODE_BDOUBLE, NULL, &ind, &dval_ptr, &len);
            return rb_float_new(*dval_ptr);
        }
    }
    rb_raise(rb_eRuntimeError, "Unsupported Typecode %d", tc);
}

static OCIAnyData *
to_return_value(plruby_context_t *ctx, VALUE val)
{
    OCIAnyData *sdata = NULL;
    OCIInd ind = OCI_IND_NOTNULL;

    if (NIL_P(val)) {
        return NULL;
    }

    switch (ctx->rettype) {
    case OCI_TYPECODE_VARCHAR2:
        {
            OCIString *str = NULL;
            VALUE sval = rb_String(val);

            OCIStringAssignText(ctx->envhp, ctx->errhp, (text*)RSTRING_PTR(sval), RSTRING_LEN(sval), &str);
            OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, str, 0, &sdata);
            OCIStringResize(ctx->envhp, ctx->errhp, 0, &str);
            return sdata;
        }
    case OCI_TYPECODE_NUMBER:
        {
            OCINumber num;
            double dval = rb_num2dbl(val);

            OCINumberFromReal(ctx->errhp, &dval, sizeof(dval), &num);
            OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, &num, 0, &sdata);
            return sdata;
        }
    case OCI_TYPECODE_BDOUBLE:
        {
            double dval = rb_num2dbl(val);

            OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, &dval, 0, &sdata);
            return sdata;
        }
    }
    rb_raise(rb_eRuntimeError, "Unsupported Typecode %d", ctx->rettype);
}

#ifdef PLRUBY_DEBUG
static void
debug_msg(const char *fmt, ...)
{
    OCIStmt *stmtp = NULL;
    OCIBind *bindp = NULL;
    OCIInd ind = 0;
    char msg[2000];
    ub2 msglen;
    const char *stmt = "BEGIN DBMS_OUTPUT.PUT_LINE(:1); END;";
    sword rv;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    msg[sizeof(msg) - 1] = '\0';
    msglen = strlen(msg);

    rv = OCIStmtPrepare2(svchp, &stmtp, errhp, (OraText*)stmt, strlen(stmt), NULL, 0, OCI_NTV_SYNTAX, OCI_DEFAULT);
    if (rv != OCI_SUCCESS) {
        return;
    }
    OCIBindByPos(stmtp, &bindp, errhp, 1, (OraText*)msg, msglen, SQLT_CHR, &ind, &msglen, NULL, 0, NULL, OCI_DEFAULT);
    OCIStmtExecute(svchp, stmtp, errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    OCIStmtRelease(stmtp, errhp, NULL, 0, OCI_DEFAULT);
}
#endif

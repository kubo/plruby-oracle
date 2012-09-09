#include <stdarg.h>
#include <string.h>
#include <ruby.h>
#include <ruby/encoding.h>
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

#define MAX_ARG 10

#define EMPTY ((void*)(-1))

#define chk(func) do { \
    sword status = func; \
    if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO) { \
        raise_oracle_error(ctx, 1, status, __FILE__, __LINE__); \
    } \
} while (0)

#define chke(func) do { \
    sword status = func; \
    if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO) { \
        raise_oracle_error(ctx, 0, status, __FILE__, __LINE__); \
    } \
} while (0)

typedef struct plruby_context {
    OCIEnv *envhp;
    OCISvcCtx *svchp;
    OCIError *errhp;
    const char *obj;
    const char *meth;
    int rettype;
    const char *argtype;
    void *args[MAX_ARG];
} plruby_context_t;

static rb_encoding *oracle_encoding;
static int trace_level = 0;

static VALUE setup_plruby_oracle(plruby_context_t *ctx);
static OCIAnyData *call_ruby(plruby_context_t *ctx);
static const char *get_error_msg(int *errnum);
static VALUE to_ruby_value(plruby_context_t *ctx, char type, void *arg, int pos);
static OCIAnyData *to_return_value(plruby_context_t *ctx, VALUE val);
static void raise_oracle_error(plruby_context_t *ctx, int is_err, sword status, const char *file, long line);
static void trace(plruby_context_t *ctx, int level, const char *fmt, ...);

PLRUBY_EXPORT void
Init_extproc_ruby(void)
{
    rb_raise(rb_eRuntimeError, "Don't require 'extproc_ruby' from ruby.");
}

static void *checkarg(int type, OCIString *v, OCINumber *n, double *d)
{
    switch (type) {
    case 'v':
        return v;
    case 'n':
        return n;
    case 'd':
        return d;
    }
    return NULL;
}

PLRUBY_EXPORT OCIAnyData *
extproc_ruby(OCIExtProcContext *with_context, short *ret_ind, int rettype,
             const char *obj, short obj_ind,
             const char *meth, short meth_ind,
             const char *argtype, int argtype_len,
             OCIString *v1, short v1_ind, OCIString *v2, short v2_ind,
             OCIString *v3, short v3_ind, OCIString *v4, short v4_ind,
             OCIString *v5, short v5_ind, OCIString *v6, short v6_ind,
             OCIString *v7, short v7_ind, OCIString *v8, short v8_ind,
             OCIString *v9, short v9_ind, OCIString *v10, short v10_ind,
             OCINumber *n1, short n1_ind, OCINumber *n2, short n2_ind,
             OCINumber *n3, short n3_ind, OCINumber *n4, short n4_ind,
             OCINumber *n5, short n5_ind, OCINumber *n6, short n6_ind,
             OCINumber *n7, short n7_ind, OCINumber *n8, short n8_ind,
             OCINumber *n9, short n9_ind, OCINumber *n10, short n10_ind,
             double d1, short d1_ind, double d2, short d2_ind,
             double d3, short d3_ind, double d4, short d4_ind,
             double d5, short d5_ind, double d6, short d6_ind,
             double d7, short d7_ind, double d8, short d8_ind,
             double d9, short d9_ind, double d10, short d10_ind)
{
    plruby_context_t ctx;
    OCIAnyData *sdata = NULL;
    static int ruby_initialized = 0;

    OCIExtProcGetEnv(with_context, &ctx.envhp, &ctx.svchp, &ctx.errhp);
    ctx.obj = obj_ind ? NULL : obj;
    ctx.meth = meth_ind ? NULL : meth;
    ctx.rettype = rettype;
    ctx.argtype = argtype;
    ctx.args[0] = argtype_len >= 1 ? checkarg(argtype[0], v1, n1, &d1) : EMPTY;
    ctx.args[1] = argtype_len >= 2 ? checkarg(argtype[1], v2, n2, &d2) : EMPTY;
    ctx.args[2] = argtype_len >= 3 ? checkarg(argtype[2], v3, n3, &d3) : EMPTY;
    ctx.args[3] = argtype_len >= 4 ? checkarg(argtype[3], v4, n4, &d4) : EMPTY;
    ctx.args[4] = argtype_len >= 5 ? checkarg(argtype[4], v5, n5, &d5) : EMPTY;
    ctx.args[5] = argtype_len >= 6 ? checkarg(argtype[5], v6, n6, &d6) : EMPTY;
    ctx.args[6] = argtype_len >= 7 ? checkarg(argtype[6], v7, n7, &d7) : EMPTY;
    ctx.args[7] = argtype_len >= 8 ? checkarg(argtype[7], v8, n8, &d8) : EMPTY;
    ctx.args[8] = argtype_len >= 9 ? checkarg(argtype[8], v9, n9, &d9) : EMPTY;
    ctx.args[9] = argtype_len >= 10 ? checkarg(argtype[9], v10, n10, &d10) : EMPTY;

    if (!ruby_initialized) {
        ruby_init();
        ruby_init_loadpath();
        ruby_script("extproc_ruby");
        ruby_initialized = 1;
    }

    {
        int state = 0;
        int plruby_initialized = 0;
        RUBY_INIT_STACK;

        if (!plruby_initialized) {
            rb_protect((VALUE(*)(VALUE))setup_plruby_oracle, (VALUE)&ctx, &state);
            if (state == 0) {
                plruby_initialized = 1;
            }
        }
        if (state == 0) {
            sdata = (OCIAnyData*)rb_protect((VALUE(*)(VALUE))call_ruby, (VALUE)&ctx, &state);
        }
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

static VALUE setup_plruby_oracle(plruby_context_t *ctx)
{
    ub2 id;
    char name[OCI_NLS_MAXBUFSZ];
    VALUE mPLRubyOracle;
    VALUE enc;

    chk(OCIAttrGet(ctx->envhp, OCI_HTYPE_ENV, &id, NULL, OCI_ATTR_ENV_CHARSET_ID, ctx->errhp));
    OCINlsCharSetIdToName(ctx->envhp, (text*)name, sizeof(name), id);

    trace(ctx, 1, "before requre 'plruby_oracle'");
    rb_require("plruby_oracle");

    trace(ctx, 1, "before eval('PLRubyOracle')");
    mPLRubyOracle = rb_eval_string("PLRubyOracle");
    trace(ctx, 2, "rb_eval_string(\"PLRubyOracle\") => %p", mPLRubyOracle);

    trace(ctx, 1, "before PLRubyOracle.init('%s')", name);
    rb_funcall(mPLRubyOracle, rb_intern("init"), 1, rb_usascii_str_new2(name));

    enc = rb_cv_get(mPLRubyOracle, "@@encoding");
    trace(ctx, 2, "rb_cv_get(mPLRubyOracle, \"@@encoding\") => %s", StringValueCStr(enc));
    oracle_encoding = rb_to_encoding(enc);
    trace(ctx, 2, "rb_enc_get(enc) => %s", rb_enc_name(oracle_encoding));
    return Qnil;
}

static OCIAnyData *
call_ruby(plruby_context_t *ctx)
{
    VALUE obj;

    trace(ctx, 1, "before encode to ('%s')", rb_enc_name(oracle_encoding));
    obj = rb_enc_str_new(ctx->obj, strlen(ctx->obj), oracle_encoding);
    trace(ctx, 1, "after encode to ('%s')", rb_enc_name(oracle_encoding));

    trace(ctx, 1, "before eval('%s')", ctx->obj);
    obj = rb_funcall(rb_cObject, rb_intern("eval"), 1, obj);
    trace(ctx, 1, "after eval('%s')", ctx->obj);
    if (ctx->meth != NULL) {
        VALUE args[MAX_ARG];
        int idx;

        for (idx = 0; idx < MAX_ARG && ctx->args[idx] != EMPTY; idx++) {
            trace(ctx, 2, "before to_ruby_value(ctx, ctx->args[%d], %d)", idx, idx);
            args[idx] = to_ruby_value(ctx, ctx->argtype[idx], ctx->args[idx], idx);
            trace(ctx, 2, "after to_ruby_value(ctx, ctx->args[%d], %d) => %p", idx, idx, args[idx]);
        }
        trace(ctx, 1, "before calling %s method", ctx->meth);
        obj = rb_funcall2(obj, rb_intern(ctx->meth), idx, args);
        trace(ctx, 1, "after calling %s method", ctx->meth);
    }
    return to_return_value(ctx, obj);
}

/*
 * Don't use the retuned value after any ruby function is called.
 * It may be GCed.
 */
static const char *
get_error_msg(int *errnum)
{
    VALUE exc = rb_String(rb_errinfo());
    VALUE num = rb_iv_get(exc, "@plsql_error_number");
    const char *errmsg;

    exc = rb_str_export_to_enc(exc, oracle_encoding);
    errmsg = StringValueCStr(exc);

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
to_ruby_value(plruby_context_t *ctx, char type, void *arg, int pos)
{
    OCIString *str;
    OCINumber *num;
    double *d;
    char *cstr;
    ub4 size;
    double dbl;

    switch (type) {
    case ' ':
        trace(ctx, 1, "arg[%d]: nil", pos);
        return Qnil;
    case 'v':
        str = (OCIString *)arg;
        cstr = (char*)OCIStringPtr(ctx->envhp, str);
        size = OCIStringSize(ctx->envhp, str);
        trace(ctx, 1, "arg[%d]: %.*s", pos, size, cstr);
        return rb_enc_str_new(cstr, size, oracle_encoding);
    case 'n':
        num = (OCINumber *)arg;
        chk(OCINumberToReal(ctx->errhp, num, sizeof(dbl), &dbl));
        trace(ctx, 1, "arg[%d]: %f", pos, dbl);
        return rb_float_new(dbl);
    case 'd':
        d = (double *)arg;
        trace(ctx, 1, "arg[%d]: %f", pos, *d);
        return rb_float_new(*d);
    }
    rb_raise(rb_eRuntimeError, "Unsupported type %d", type);
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
            sval = rb_str_export_to_enc(sval, oracle_encoding);

            chk(OCIStringAssignText(ctx->envhp, ctx->errhp, (text*)RSTRING_PTR(sval), RSTRING_LEN(sval), &str));
            chk(OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, str, 0, &sdata));
            chk(OCIStringResize(ctx->envhp, ctx->errhp, 0, &str));
            return sdata;
        }
    case OCI_TYPECODE_NUMBER:
        {
            OCINumber num;
            double dval = rb_num2dbl(val);

            chk(OCINumberFromReal(ctx->errhp, &dval, sizeof(dval), &num));
            chk(OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, &num, 0, &sdata));
            return sdata;
        }
    case OCI_TYPECODE_BDOUBLE:
        {
            double dval = rb_num2dbl(val);

            chk(OCIAnyDataConvert(ctx->svchp, ctx->errhp, ctx->rettype, NULL, OCI_DURATION_SESSION, &ind, &dval, 0, &sdata));
            return sdata;
        }
    }
    rb_raise(rb_eRuntimeError, "Unsupported Typecode %d", ctx->rettype);
}

static void
raise_oracle_error(plruby_context_t *ctx, int is_err, sword status, const char *file, long line)
{
    text errmsg[OCI_ERROR_MAXMSG_SIZE];
    sb4 errcode;

    switch (status) {
    case OCI_ERROR:
    case OCI_SUCCESS_WITH_INFO:
    case OCI_NO_DATA:
        OCIErrorGet(is_err ? ctx->errhp : ctx->envhp, 1, NULL, &errcode, errmsg, sizeof(errmsg),
                    is_err ? OCI_HTYPE_ERROR : OCI_HTYPE_ENV);
        rb_raise(rb_eRuntimeError, "OCI Error(%s:%ld): %s", file, line, errmsg);
        break;
    case OCI_INVALID_HANDLE:
        rb_raise(rb_eRuntimeError, "OCI Error(%s:%ld): Invalid Handle", file, line);
        break;
    case OCI_NEED_DATA:
        rb_raise(rb_eRuntimeError, "OCI Error(%s:%ld): Need Data", file, line);
        break;
    case OCI_STILL_EXECUTING:
        rb_raise(rb_eRuntimeError, "OCI Error(%s:%ld): Still Executing", file, line);
        break;
    case OCI_CONTINUE:
        rb_raise(rb_eRuntimeError, "OCI Error(%s:%ld): Continue", file, line);
        break;
    default:
        rb_raise(rb_eRuntimeError, "OCI error(%s:%ld): Unknown code %d", file, line, status);
    }
}

static void
trace(plruby_context_t *ctx, int level, const char *fmt, ...)
{
    OCIStmt *stmtp = NULL;
    OCIBind *bindp = NULL;
    OCIInd ind = 0;
    char msg[2000];
    ub2 msglen;
    const char *stmt = "BEGIN DBMS_OUTPUT.PUT_LINE(:1); END;";
    sword rv;
    va_list ap;

    if (level > trace_level) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    msg[sizeof(msg) - 1] = '\0';
    msglen = strlen(msg);

    rv = OCIStmtPrepare2(ctx->svchp, &stmtp, ctx->errhp, (OraText*)stmt, strlen(stmt), NULL, 0, OCI_NTV_SYNTAX, OCI_DEFAULT);
    if (rv != OCI_SUCCESS) {
        return;
    }
    OCIBindByPos(stmtp, &bindp, ctx->errhp, 1, (OraText*)msg, msglen, SQLT_CHR, &ind, &msglen, NULL, 0, NULL, OCI_DEFAULT);
    OCIStmtExecute(ctx->svchp, stmtp, ctx->errhp, 1, 0, NULL, NULL, OCI_DEFAULT);
    OCIStmtRelease(stmtp, ctx->errhp, NULL, 0, OCI_DEFAULT);
}

CREATE OR REPLACE PACKAGE ruby
IS
  FUNCTION call0(rettype PLS_INTEGER, obj VARCHAR2, meth VARCHAR2, argtype VARCHAR2,
    v1 VARCHAR2, v2 VARCHAR2, v3 VARCHAR2, v4 VARCHAR2, v5 VARCHAR2,
    v6 VARCHAR2, v7 VARCHAR2, v8 VARCHAR2, v9 VARCHAR2, v10 VARCHAR2,
    n1 NUMBER, n2 NUMBER, n3 NUMBER, n4 NUMBER, n5 NUMBER,
    n6 NUMBER, n7 NUMBER, n8 NUMBER, n9 NUMBER, n10 NUMBER,
    d1 BINARY_DOUBLE, d2 BINARY_DOUBLE, d3 BINARY_DOUBLE, d4 BINARY_DOUBLE, d5 BINARY_DOUBLE,
    d6 BINARY_DOUBLE, d7 BINARY_DOUBLE, d8 BINARY_DOUBLE, d9 BINARY_DOUBLE, d10 BINARY_DOUBLE)
    RETURN ANYDATA;

  FUNCTION call_s(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN VARCHAR2;

  FUNCTION call_n(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN NUMBER;

  FUNCTION call_f(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN BINARY_DOUBLE;
END;
/

CREATE OR REPLACE PACKAGE BODY ruby
IS
  FUNCTION call0(rettype PLS_INTEGER, obj VARCHAR2, meth VARCHAR2, argtype VARCHAR2,
    v1 VARCHAR2, v2 VARCHAR2, v3 VARCHAR2, v4 VARCHAR2, v5 VARCHAR2,
    v6 VARCHAR2, v7 VARCHAR2, v8 VARCHAR2, v9 VARCHAR2, v10 VARCHAR2,
    n1 NUMBER, n2 NUMBER, n3 NUMBER, n4 NUMBER, n5 NUMBER,
    n6 NUMBER, n7 NUMBER, n8 NUMBER, n9 NUMBER, n10 NUMBER,
    d1 BINARY_DOUBLE, d2 BINARY_DOUBLE, d3 BINARY_DOUBLE, d4 BINARY_DOUBLE, d5 BINARY_DOUBLE,
    d6 BINARY_DOUBLE, d7 BINARY_DOUBLE, d8 BINARY_DOUBLE, d9 BINARY_DOUBLE, d10 BINARY_DOUBLE)
    RETURN ANYDATA
  IS LANGUAGE C LIBRARY extproc_ruby_lib NAME "extproc_ruby" WITH CONTEXT
  PARAMETERS
  (CONTEXT, RETURN INDICATOR, rettype, obj, obj INDICATOR, meth, meth INDICATOR, argtype, argtype LENGTH,
   v1 OCISTRING, v1 INDICATOR, v2 OCISTRING, v2 INDICATOR, v3 OCISTRING, v3 INDICATOR, v4 OCISTRING, v4 INDICATOR, v5 OCISTRING, v5 INDICATOR,
   v6 OCISTRING, v6 INDICATOR, v7 OCISTRING, v7 INDICATOR, v8 OCISTRING, v8 INDICATOR, v9 OCISTRING, v9 INDICATOR, v10 OCISTRING, v10 INDICATOR,
   n1, n1 INDICATOR, n2, n2 INDICATOR, n3, n3 INDICATOR, n4, n4 INDICATOR, n5, n5 INDICATOR,
   n6, n6 INDICATOR, n7, n7 INDICATOR, n8, n8 INDICATOR, n9, n9 INDICATOR, n10, n10 INDICATOR,
   d1, d1 INDICATOR, d2, d2 INDICATOR, d3, d3 INDICATOR, d4, d4 INDICATOR, d5, d5 INDICATOR,
   d6, d6 INDICATOR, d7, d7 INDICATOR, d8, d8 INDICATOR, d9, d9 INDICATOR, d10, d10 INDICATOR);

  FUNCTION call_a(rettype PLS_INTEGER, obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN ANYDATA
  IS
    argtype VARCHAR2(10) := '';
    v1 VARCHAR2(4000); v2 VARCHAR2(4000); v3 VARCHAR2(4000); v4 VARCHAR2(4000); v5 VARCHAR2(4000);
    v6 VARCHAR2(4000); v7 VARCHAR2(4000); v8 VARCHAR2(4000); v9 VARCHAR2(4000); v10 VARCHAR2(4000);
    n1 NUMBER; n2 NUMBER; n3 NUMBER; n4 NUMBER; n5 NUMBER;
    n6 NUMBER; n7 NUMBER; n8 NUMBER; n9 NUMBER; n10 NUMBER;
    d1 BINARY_DOUBLE; d2 BINARY_DOUBLE; d3 BINARY_DOUBLE; d4 BINARY_DOUBLE; d5 BINARY_DOUBLE;
    d6 BINARY_DOUBLE; d7 BINARY_DOUBLE; d8 BINARY_DOUBLE; d9 BINARY_DOUBLE; d10 BINARY_DOUBLE;

    FUNCTION checkarg(a ANYDATA, v OUT VARCHAR2, n OUT NUMBER, d OUT BINARY_DOUBLE) RETURN CHAR
    IS
      typecode PLS_INTEGER;
      type_ AnyType;
    BEGIN
      IF a1 IS NULL THEN
        RETURN ' ';
      ELSE
        typecode := a.GetType(type_);
        CASE typecode
        WHEN DBMS_TYPES.TYPECODE_VARCHAR2 THEN
          v := a.AccessVarchar2();
          RETURN 'v';
        WHEN DBMS_TYPES.TYPECODE_NUMBER THEN
          n := a.AccessNumber();
          RETURN 'n';
        WHEN DBMS_TYPES.TYPECODE_BDOUBLE THEN
          d := a.AccessBDouble();
          RETURN 'd';
        ELSE
          RAISE_APPLICATION_ERROR(-20999, 'Unsupported type ' || a.GetTypeName());
        END CASE;
      END IF;
    END;
  BEGIN
    IF narg >= 1 THEN
      argtype := argtype || checkarg(a1, v1, n1, d1);
    END IF;
    IF narg >= 2 THEN
      argtype := argtype || checkarg(a2, v2, n2, d2);
    END IF;
    IF narg >= 3 THEN
      argtype := argtype || checkarg(a3, v3, n3, d3);
    END IF;
    IF narg >= 4 THEN
      argtype := argtype || checkarg(a4, v4, n4, d4);
    END IF;
    IF narg >= 5 THEN
      argtype := argtype || checkarg(a5, v5, n5, d5);
    END IF;
    IF narg >= 6 THEN
      argtype := argtype || checkarg(a6, v6, n6, d6);
    END IF;
    IF narg >= 7 THEN
      argtype := argtype || checkarg(a7, v7, n7, d7);
    END IF;
    IF narg >= 8 THEN
      argtype := argtype || checkarg(a8, v8, n8, d8);
    END IF;
    IF narg >= 9 THEN
      argtype := argtype || checkarg(a9, v9, n9, d9);
    END IF;
    IF narg >= 10 THEN
      argtype := argtype || checkarg(a10, v10, n10, d10);
    END IF;
    RETURN call0(rettype, obj, meth, argtype,
                 v1, v2 ,v3, v4, v5, v6, v7, v8, v9, v10,
                 n1, n2 ,n3, n4, n5, n6, n7, n8, n9, n10,
                 d1, d2 ,d3, d4, d5, d6, d7, d8, d9, d10);
  END;

  FUNCTION call_s(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN VARCHAR2
  IS
    rval ANYDATA;
  BEGIN
    rval := call_a(DBMS_TYPES.TYPECODE_VARCHAR2, obj, meth, narg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    IF rval IS NOT NULL THEN
      RETURN rval.AccessVarchar2();
    ELSE
      RETURN NULL;
    END IF;
  END;

  FUNCTION call_n(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN NUMBER
  IS
    rval ANYDATA;
  BEGIN
    rval := call_a(DBMS_TYPES.TYPECODE_NUMBER, obj, meth, narg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    IF rval IS NOT NULL THEN
      RETURN rval.AccessNumber();
    ELSE
      RETURN NULL;
    END IF;
  END;

  FUNCTION call_f(obj VARCHAR2, meth VARCHAR2 := NULL, narg PLS_INTEGER := NULL,
    a1 ANYDATA := NULL, a2 ANYDATA := NULL, a3 ANYDATA := NULL,
    a4 ANYDATA := NULL, a5 ANYDATA := NULL, a6 ANYDATA := NULL,
    a7 ANYDATA := NULL, a8 ANYDATA := NULL, a9 ANYDATA := NULL,
    a10 ANYDATA := NULL) RETURN BINARY_DOUBLE
  IS
    rval ANYDATA;
  BEGIN
    rval := call_a(DBMS_TYPES.TYPECODE_BDOUBLE, obj, meth, narg, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    IF rval IS NOT NULL THEN
      RETURN rval.AccessBDouble();
    ELSE
      RETURN NULL;
    END IF;
  END;
END;
/
show error

/* valconv.h */

#define SECURE(obj) SecureObject((obj), #obj)

#define SHP_NODATA_LIM	-1.0e-38
#define SHP_NODATA	-2.0e-38
#define SHP_NUM2DBL(value) (NIL_P(value) ? SHP_NODATA : NUM2DBL(value))
#define SHP_FLOAT_NEW(f) (((f) < SHP_NODATA_LIM) ? Qnil : rb_float_new((f)))
#define CSTR2SYM(s) (ID2SYM(rb_intern((s))))
static int NUM2INT_nil(VALUE v) { return NIL_P(v) ? -1 : NUM2INT(v); }

static const_table_entry sl_consts[] = {
    { "String", INT2FIX((int)FTString) },
    { "Integer", INT2FIX((int)FTInteger) },
    { "Float", INT2FIX((int)FTDouble) },
    { "Logical", INT2FIX((int)FTLogical) },
    { NULL, Qnil }
};

static ID *ShapeTypeNameTable(void)
{
    static ID ids[SHPT_MULTIPATCH + 1];
    if (ids[SHPT_NULL] != 0) {
	return ids;
    }
    ids[SHPT_NULL] = rb_intern("Null");
    ids[SHPT_POINT] = rb_intern("Point");
    ids[SHPT_POINTM] = rb_intern("PointM");
    ids[SHPT_POINTZ] = rb_intern("PointZ");
    ids[SHPT_ARC] = rb_intern("Arc");
    ids[SHPT_ARCM] = rb_intern("ArcM");
    ids[SHPT_ARCZ] = rb_intern("ArcZ");
    ids[SHPT_POLYGON] = rb_intern("Polygon");
    ids[SHPT_POLYGONM] = rb_intern("PolygonM");
    ids[SHPT_POLYGONZ] = rb_intern("PolygonZ");
    ids[SHPT_MULTIPOINT] = rb_intern("MultiPoint");
    ids[SHPT_MULTIPOINTM] = rb_intern("MultiPointM");
    ids[SHPT_MULTIPOINTZ] = rb_intern("MultiPointZ");
    ids[SHPT_MULTIPATCH] = rb_intern("MultiPatch");
    return ids;
}

static ID *FieldTypeNameTable(void)
{
    static ID ids[FTInvalid + 1];
    if (ids[FTString] != 0) {
	return ids;
    }
    ids[FTString] = rb_intern("String");
    ids[FTInteger] = rb_intern("Integer");
    ids[FTDouble] = rb_intern("Float");
    ids[FTLogical] = rb_intern("Logical");
    ids[FTInvalid] = rb_intern("Invalid");
    return ids;
}

#define SHPP_RING 5

static ID *PartTypeNameTable(void)
{
    static ID ids[6];
    if (ids[0] != 0) {
	return ids;
    }
    ids[0] = rb_intern("TriangleStrip");
    ids[1] = rb_intern("TriangleFan");
    ids[2] = rb_intern("OuterRing");
    ids[3] = rb_intern("InnerRing");
    ids[4] = rb_intern("FirstRing");
    ids[5] = rb_intern("Ring");
    return ids;
}

static int Ruby2PartType(VALUE ptype)
{
    ID given, *ids;
    int i;
    switch (TYPE(ptype)) {
	case T_STRING:
	case T_SYMBOL:
	    given = rb_to_id(ptype);
	    ids = PartTypeNameTable();
	    for (i = 0; i < 6; i++) {
		if (ids[i] == given)
		    return i;
	    }
	    return -1;
	    break;
	case T_FIXNUM:
	    return FIX2INT(ptype);
	default:
	    return NUM2INT(ptype);
    }
}

static VALUE PartType2Ruby(int part_type)
{
    if (part_type < 0 || part_type > 5) {
	return INT2NUM(part_type);
    } else {
	ID *ids;
	ids = PartTypeNameTable();
	return ID2SYM(ids[part_type]);
    }
}

static DBFFieldType Ruby2FieldType(VALUE obj)
{
    if (FIXNUM_P(obj)) {
	int ft = FIX2INT(obj);
	switch (ft) {
	case FTString:
	case FTInteger:
	case FTDouble:
	case FTLogical:
	case FTInvalid:
	    return ft;
	default:
	    rb_raise(rb_eArgError, "bad field_type %d", ft);
	}
    } else if (TYPE(obj) == T_STRING || TYPE(obj) == T_SYMBOL) {
	ID given, *table;
	int i;
	given = rb_to_id(obj);
	table = FieldTypeNameTable();
	for (i = 0; i < FTInvalid; i++) {
	    if (given == table[i])
		return i;
	}
    }
    rb_raise(rb_eArgError, "bad field_type");
}

static VALUE FieldType2Ruby(DBFFieldType ftype)
{
    int ft;
    ft = ftype;
    if (ft < 0 || ft >= FTInvalid)
	return INT2FIX(ft);
    return ID2SYM(FieldTypeNameTable()[ft]);
}

static VALUE ShapeType2Class(int nSHPType, VALUE klass)
{
    switch (nSHPType) {
    case SHPT_POINT:		return cPoint;
    case SHPT_POINTM:		return cPointM;
    case SHPT_POINTZ:		return cPointZ;
    case SHPT_ARC:		return cArc;
    case SHPT_ARCM:		return cArcM;
    case SHPT_ARCZ:		return cArcZ;
    case SHPT_POLYGON:		return cPolygon;
    case SHPT_POLYGONM:		return cPolygonM;
    case SHPT_POLYGONZ:		return cPolygonZ;
    case SHPT_MULTIPOINT:	return cMultiPoint;
    case SHPT_MULTIPOINTM:	return cMultiPointM;
    case SHPT_MULTIPOINTZ:	return cMultiPointZ;
    case SHPT_MULTIPATCH:	return cMultiPatch;
    case SHPT_NULL:
    default:
	return klass;
    }
}

static VALUE ShapeType2Ruby(int nSHPType)
{
    ID *ids;
    switch (nSHPType) {
    case SHPT_NULL:
    case SHPT_POINT:	case SHPT_POINTM:	case SHPT_POINTZ:
    case SHPT_ARC:	case SHPT_ARCM:		case SHPT_ARCZ:
    case SHPT_POLYGON:	case SHPT_POLYGONM:	case SHPT_POLYGONZ:
    case SHPT_MULTIPOINT: case SHPT_MULTIPOINTM: case SHPT_MULTIPOINTZ:
    case SHPT_MULTIPATCH:
	ids = ShapeTypeNameTable();
	return ID2SYM(ids[nSHPType]);
    default:
	return INT2FIX(nSHPType);
    }
}

static int ClassIs(VALUE a, VALUE b)
{
    VALUE r;
    static ID le = 0;
    if (le == 0) {
	le = rb_intern("<=");
    }
    r = rb_funcall(a, le, 1, b);
    switch (TYPE(r)) {
    case T_TRUE:
	return 1;
    case T_FALSE:
    case T_NIL:
	return 0;
    default:
	return -1;
    }
}

static int ShapeClass_to_Int(VALUE klass)
{
    if (ClassIs(klass, cPoint)) {
	if (ClassIs(klass, cPointZ)) {
	    return SHPT_POINTZ;
	} else if (ClassIs(klass, cPointM)) {
	    return SHPT_POINTM;
	}
	return SHPT_POINT;
    } else if (ClassIs(klass, cArc)) {
	if (ClassIs(klass, cArcZ)) {
	    return SHPT_ARCZ;
	} else if (ClassIs(klass, cArcM)) {
	    return SHPT_ARCM;
	}
	return SHPT_ARC;
    } else if (ClassIs(klass, cPolygon)) {
	if (ClassIs(klass, cPolygonZ)) {
	    return SHPT_POLYGONZ;
	} else if (ClassIs(klass, cPolygonM)) {
	    return SHPT_POLYGONM;
	}
	return SHPT_POLYGON;
    } else if (ClassIs(klass, cMultiPoint)) {
	if (ClassIs(klass, cMultiPointZ)) {
	    return SHPT_MULTIPOINTZ;
	} else if (ClassIs(klass, cMultiPointM)) {
	    return SHPT_MULTIPOINTM;
	}
	return SHPT_MULTIPOINT;
    } else if (ClassIs(klass, cMultiPatch)) {
	return SHPT_MULTIPATCH;
    } else {
	rb_raise(rb_eArgError, "invalid class");
    }
}

static int ShapeSym_to_Int(VALUE stype)
{
    ID arg, *ids;
    int i;
    ids = ShapeTypeNameTable();
    arg = rb_to_id(stype);
    for (i = 0; i <= SHPT_MULTIPATCH; i++) {
	if (arg == ids[i])
	    return i;
    }
    rb_raise(rb_eArgError, "invalid shape class name");
    return -1;
}

static int Ruby2ShapeType(VALUE stype)
{
    switch (TYPE(stype)) {
    case T_FIXNUM:
	return FIX2INT(stype);
    case T_CLASS:
	return ShapeClass_to_Int(stype);
    case T_STRING:
    case T_SYMBOL:
	return ShapeSym_to_Int(stype);
    default:
	rb_raise(rb_eArgError, "shape_type is Integer/String/Symbol/Class");
    }
}

static VALUE DoubleUnpack(double *dp, unsigned num)
{
    VALUE ary;
    int i;
    ary = rb_ary_new2(num);
    for (i = 0; i < num; i++) {
	rb_ary_push(ary, rb_float_new(dp[i]));
    }
    return ary;
}

static VALUE DoubleUnpackM(double *dp, unsigned num, int mpos)
{
    VALUE ary;
    int i;
    ary = rb_ary_new2(num);
    for (i = 0; i < num; i++) {
	if ((mpos == -1) || (i == mpos)) {
	    rb_ary_push(ary, SHP_FLOAT_NEW(dp[i]));
	} else {
	    rb_ary_push(ary, rb_float_new(dp[i]));
	}
    }
    return ary;
}

static VALUE IntUnpack(int *up, unsigned num)
{
    static ID unpackid;
    static VALUE unpackfmt = 0;
    VALUE buf;
    if (unpackfmt == 0) {
			unpackid = rb_intern("unpack");
			unpackfmt = rb_str_new2("i*");
                        rb_global_variable(&unpackfmt);
    }
    buf = rb_str_new((char *)up, sizeof(int) * num);
    return rb_funcall(buf, unpackid, 1, unpackfmt);
}

static VALUE SecureObject(VALUE obj, const char *name) {
    if (OBJ_TAINTED(obj) && ruby_safe_level >= 1) {
	rb_raise(rb_eSecurityError, "tainted %s", name);
    }
    return obj;
}


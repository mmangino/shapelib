/*
 * constructors
 */
#define NUM2DBLo(val) (NIL_P((val)) ? 0.0 : NUM2DBL((val)))

static int *IntPack(VALUE array, int decode_p)
{
    int *buf;
    int i;
    Check_Type(array, T_ARRAY);
    buf = xmalloc(sizeof(int) * RARRAY(array)->len);
    for (i = 0; i < RARRAY(array)->len; i++) {
	if (decode_p)
	    buf[i] = Ruby2PartType(RARRAY(array)->ptr[i]);
	else
	    buf[i] = NUM2INT(RARRAY(array)->ptr[i]);
    }
    return buf;
}

static double *DoublePack(VALUE array, double nilval)
{
    double *buf;
    int i;
    if (NIL_P(array))
	return NULL;
    Check_Type(array, T_ARRAY);
    buf = xmalloc(sizeof(double) * RARRAY(array)->len);
    for (i = 0; i < RARRAY(array)->len; i++) {
	VALUE v;
	v = RARRAY(array)->ptr[i];
	buf[i] = (NIL_P(v) ? nilval : NUM2DBL(RARRAY(array)->ptr[i]));
    }
    return buf;
}

static VALUE HashDeref(VALUE hash, const char *key)
{
    if (T_HASH == TYPE(hash)) {
	return rb_hash_aref(hash, CSTR2SYM(key));
    } else {
	static ID op = 0;
	if (op == 0)
	    op = rb_intern("[]");
	return rb_funcall(hash, op, CSTR2SYM(key));
    }
}

static VALUE sp_field_set(VALUE obj, VALUE name, VALUE val);

static void HashLoadAttrib(VALUE vshape, VALUE hash)
{
    VALUE ary, *argv;
    ID to_a = rb_intern("to_a");
    int i;
    if (!rb_respond_to(hash, to_a))
	return;
    ary = rb_funcall(hash, to_a, 0);
    if (T_ARRAY != TYPE(ary))
	return;
    argv = RARRAY(ary)->ptr;
    for (i = 0; i < RARRAY(ary)->len; i++) {
	VALUE pair = argv[i];
	VALUE key, val;
	if (T_ARRAY != TYPE(pair))
	    continue;
	if (RARRAY(pair)->len < 2)
	    continue;
	key = RARRAY(pair)->ptr[0];
	val = RARRAY(pair)->ptr[1];
	if (T_STRING != TYPE(key))
	    continue;
	sp_field_set(vshape, key, val);
    }
}


static VALUE sp_s_new(VALUE klass, VALUE hash)
{
    shape_t *shape;
    VALUE qlass, vshape;
    int shape_type, shape_id, n_parts, n_vertices;
    int *part_type, *part_start;
    double *xvals, *yvals, *zvals, *mvals;
    shape_type = Ruby2ShapeType(HashDeref(hash, "shape_type"));
    qlass = ShapeType2Class(shape_type, klass);
    shape_id = NUM2INT_nil(HashDeref(hash, "shape_id"));
    n_parts = NUM2INT(HashDeref(hash, "n_parts"));
    n_vertices = NUM2INT(HashDeref(hash, "n_vertices"));
    part_type = IntPack(HashDeref(hash, "part_type"), 1);
    part_start = IntPack(HashDeref(hash, "part_start"), 0);
    xvals = DoublePack(HashDeref(hash, "xvals"), 0.0);
    yvals = DoublePack(HashDeref(hash, "yvals"), 0.0);
    zvals = DoublePack(HashDeref(hash, "zvals"), 0.0);
    mvals = DoublePack(HashDeref(hash, "mvals"), SHP_NODATA);
    if (zvals == NULL) rb_raise(rb_eRuntimeError, "missing zvals");
    if (mvals == NULL) rb_raise(rb_eRuntimeError, "missing mvals");
    shape = shape_new(shape_type, shape_id,
	n_parts, part_type, part_start, n_vertices,
	xvals, yvals, zvals, mvals);
    if (shape == NULL) rb_raise(rb_eRuntimeError, "SHPCreateObject fails");
    free(part_type);
    free(part_start);
    if (xvals) free(xvals);
    if (yvals) free(yvals);
    if (zvals) free(zvals);
    if (mvals) free(mvals);
    vshape = Data_Wrap_Struct(qlass, 0, shape_close, shape);
    HashLoadAttrib(vshape, hash);
    return vshape;
}

static int CheckPartSize(int argc, VALUE *argv, int *part_start,
    int *part_type)
{    
    int pstart = 0;
    int i;
    for (i = 0; i < argc; i++) {
	VALUE first;
	int partlen;
	if (T_ARRAY != TYPE(argv[i])) {
	    rb_raise(rb_eArgError, "arg %d must be Array", i);
	    return -1;
	}
	partlen = RARRAY(argv[i])->len;
	first = RARRAY(argv[i])->ptr[0];
	switch (TYPE(first)) {
	    case T_SYMBOL: case T_STRING:
		part_type[i] = Ruby2PartType(first);
		partlen--;
		break;
	    default:
		part_type[i] = SHPP_RING;
		;
	}
	part_start[i] = pstart;
	pstart += partlen;
    }
    /* returns total number of vertices */
    return pstart;
}

static void ExtractXYMZ(VALUE v,
    double *x, double *y, double *m, double *z)
{
    static int idx = 0, idy, idz, idm;
    if (idx == 0) {
	idx = rb_intern("x"); idy = rb_intern("y");
	idm = rb_intern("m"); idz = rb_intern("z");
    }
    if (T_ARRAY == TYPE(v)) {
	int argc = RARRAY(v)->len;
	VALUE *argv = RARRAY(v)->ptr;
	*x = (argc > 0) ? NUM2DBLo(argv[0]) : 0.0;
	*y = (argc > 1) ? NUM2DBLo(argv[1]) : 0.0;
	*m = SHP_NUM2DBL((argc > 2) ? argv[2] : Qnil);
	*z = (argc > 3) ? NUM2DBL(argv[3]) : 0.0;
    } else {
	*x = NUM2DBLo(rb_funcall(v, idx, 0));
	*y = NUM2DBLo(rb_funcall(v, idy, 0));
	if (rb_respond_to(v, idm)) {
	    *m = SHP_NUM2DBL(rb_funcall(v, idm, 0));
	} else {
	    *m = SHP_NODATA;
	}
	if (rb_respond_to(v, idz)) {
	    *z = NUM2DBLo(rb_funcall(v, idz, 0));
	} else {
	    *z = 0.0;
	}
    }
}

static void CollectPoints(int argc, const VALUE *argv,
    double *x, double *y, double *m, double *z)
{
    int i, ofs;
    ofs = 0;
    if (argc > 0) {
	switch (TYPE(argv[0])) {
	case T_SYMBOL: case T_STRING:
	    ofs++;
	}
    }
    for (i = 0; i < argc - ofs; i++) {
	ExtractXYMZ(argv[i + ofs], x + i, y + i, m + i, z + i);
    }
}

static shape_t *NewMultiPart(int argc, VALUE *argv, VALUE klass, int shapetype)
{
    shape_t *shape;
    double *dblock, *x, *y, *m, *z;
    int *part_start, *part_type;
    int n_parts, n_vert;
    int ipart;

    n_parts = argc;
    part_start = xmalloc(sizeof(int) * n_parts);
    part_type = xmalloc(sizeof(int) * n_parts);
    n_vert = CheckPartSize(argc, argv, part_start, part_type);
    dblock = xmalloc(sizeof(double) * n_vert * 4);
    x = dblock;
    y = dblock + n_vert;
    m = dblock + n_vert * 2;
    z = dblock + n_vert * 3;
    for (ipart = 0; ipart < argc; ipart++) {
	CollectPoints(
	    RARRAY(argv[ipart])->len, RARRAY(argv[ipart])->ptr,
	    x + part_start[ipart], y + part_start[ipart],
	    m + part_start[ipart], z + part_start[ipart]);
    }
    shape = shape_new_obj(shapetype,
	n_parts, part_start, part_type, n_vert,
	x, y, m, z);
    free(dblock);
    free(part_start);
    free(part_type);
    return shape;
}

static shape_t *NewSinglePart(int argc, VALUE *argv, VALUE klass, int shapetype)
{
    shape_t *shape;
    double *dblock, *x, *y, *m, *z;
    int n_vert, part_type_buf;
    int part_start_buf = 0;
    n_vert = argc;
    switch (TYPE(argv[0])) {
	case T_STRING: case T_SYMBOL:
	    part_type_buf = Ruby2PartType(argv[0]);
	    n_vert--;
	    break;
	default:
	    part_type_buf = SHPP_RING;
    }
    dblock = xmalloc(sizeof(double) * n_vert * 4);
    x = dblock;
    y = dblock + n_vert;
    m = dblock + n_vert * 2;
    z = dblock + n_vert * 3;
    CollectPoints(argc, argv, x, y, m, z);
    shape = shape_new_obj(shapetype,
	1, &part_start_buf, &part_type_buf, n_vert, x, y, m, z);
    free(dblock);
    return shape;
}

/*
 * == Single Part
 *  new [1, 2], [3, 4], ...
 *  new point, point, ...
 *  new :PartType, point, point, ...
 * == Multi Part
 *  new [[1, 2], [3, 4], ...], ...
 *  new [point, point, ...], ...
 *  new [:PartType, point, point, ...], ...
 */

static VALUE NewShape(int argc, VALUE *argv, VALUE klass, int shapetype)
{
    shape_t *shape;
    int multipart;
    monitor(("NewShape"));
    if (T_ARRAY == TYPE(argv[0])) {
	switch (TYPE( RARRAY(argv[0])->ptr[0] )) {
	case T_FIXNUM:
	case T_FLOAT:
	    multipart = 0;
	    break;
	default:
	    multipart = 1;
	    break;
	}
    } else {
	multipart = 0;
    }
    if (multipart) {
	shape = NewMultiPart(argc, argv, klass, shapetype);
    } else {
	shape = NewSinglePart(argc, argv, klass, shapetype);
    }
    return Data_Wrap_Struct(klass, 0, shape_close, shape);
}

static VALUE MultiPointNew(int argc, VALUE *argv, VALUE klass, int shapetype)
{
    shape_t *shape;
    double *dblock, *x, *y, *m, *z;
    int *part_start;
    int n_vert;
    int part_start_buf = 0;
    part_start = &part_start_buf;
    n_vert = argc;
    dblock = malloc(sizeof(double) * n_vert * 4);
    x = dblock;
    y = dblock + n_vert;
    m = dblock + n_vert * 2;
    z = dblock + n_vert * 3;
    CollectPoints(argc, argv, x, y, m, z);
    shape = shape_new_obj(shapetype, 1, part_start, NULL, n_vert, x, y, m, z);
    free(dblock);
    return Data_Wrap_Struct(klass, 0, shape_close, shape);
}

static VALUE arc_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_ARC);
}

static VALUE arcm_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_ARCM);
}

static VALUE arcz_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_ARCZ);
}

static VALUE pl_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_POLYGON);
}

static VALUE plm_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_POLYGONM);
}

static VALUE plz_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_POLYGONZ);
}

static VALUE mpatch_s_new(int argc, VALUE *argv, VALUE klass) {
    return NewShape(argc, argv, klass, SHPT_MULTIPATCH);
}

static VALUE mp_s_new(int argc, VALUE *argv, VALUE klass) {
    return MultiPointNew(argc, argv, klass, SHPT_MULTIPOINT);
}

static VALUE mpm_s_new(int argc, VALUE *argv, VALUE klass) {
    return MultiPointNew(argc, argv, klass, SHPT_MULTIPOINTM);
}

static VALUE mpz_s_new(int argc, VALUE *argv, VALUE klass) {
    return MultiPointNew(argc, argv, klass, SHPT_MULTIPOINTZ);
}


static void PointAddAttr(VALUE pt, VALUE attrs)
{
    VALUE *argv;
    VALUE ary;
    int i, argc;
    Check_Type(attrs, T_HASH);
    ary = rb_funcall(attrs, rb_intern("to_a"), 0);
    Check_Type(ary, T_ARRAY);
    argv = RARRAY(ary)->ptr;
    argc = RARRAY(ary)->len;
    for (i = 0; i < argc; i++) {
	VALUE *pair;
	Check_Type(argv[i], T_ARRAY);
	if (RARRAY(argv[i])->len != 2)
	    rb_bug("hash.to_a member has weird size");
	pair = RARRAY(argv[i])->ptr;
	sp_field_set(pt, pair[0], pair[1]);
    }
}


static VALUE pt_s_new(int argc, VALUE *argv, VALUE klass)
{
    shape_t *shape;
    VALUE vx, vy, attrs, pt;
    rb_scan_args(argc, argv, "21", &vx, &vy, &attrs);
    shape = shape_new_point(SHPT_POINT,
	NUM2DBLo(vx), NUM2DBLo(vy), 0.0, 0.0);
    pt = Data_Wrap_Struct(klass, 0, shape_close, shape);
    if (!NIL_P(attrs)) {
	PointAddAttr(pt, attrs);
    }
    return pt;
}

static VALUE ptm_s_new(int argc, VALUE *argv, VALUE klass)
{
    shape_t *shape;
    VALUE vx, vy, vm, attrs, pt;
    rb_scan_args(argc, argv, "31", &vx, &vy, &vm, &attrs);
    shape = shape_new_point(SHPT_POINTM,
	NUM2DBLo(vx), NUM2DBLo(vy), SHP_NUM2DBL(vm), 0.0);
    pt = Data_Wrap_Struct(klass, 0, shape_close, shape);
    if (!NIL_P(attrs)) {
	PointAddAttr(pt, attrs);
    }
    return pt;
}

static VALUE ptz_s_new(int argc, VALUE *argv, VALUE klass)
{
    shape_t *shape;
    VALUE vx, vy, vm, vz, attrs, pt;
    rb_scan_args(argc, argv, "41", &vx, &vy, &vm, &vz, &attrs);
    shape = shape_new_point(SHPT_POINTZ,
	NUM2DBLo(vx), NUM2DBLo(vy), SHP_NUM2DBL(vm), NUM2DBLo(vz));
    pt = Data_Wrap_Struct(klass, 0, shape_close, shape);
    if (!NIL_P(attrs)) {
	PointAddAttr(pt, attrs);
    }
    return pt;
}

/* ShapeLib.new_point */
static VALUE sl_m_new_point(int argc, VALUE *argv, VALUE klass)
{
    if (2 == argc || (3 == argc && T_HASH == TYPE(argv[2]))) {
	return pt_s_new(argc, argv, cPoint);
    } else if (3 == argc || (4 == argc && T_HASH != TYPE(argv[3]))) {
	return ptm_s_new(argc, argv, cPointM);
    } else if (4 == argc || (5 == argc && T_HASH != TYPE(argv[4]))) {
	return ptz_s_new(argc, argv, cPointZ);
    } else {
	rb_raise(rb_eArgError,
	"usage: ShapeLib::Shape.new_point x, y, m, z, attr_array");
	return Qnil;
    }
}

/*
 * instance methods of general Shape
 */

static void Attr_to_Hash(VALUE h, VALUE obj)
{
    shape_t *self;
    int i;
    Data_Get_Struct(obj, shape_t, self);
    if (self->nattr <= 0)
	return;
    for (i = 0; i < self->nattr; i++) {
	attrib_t *a;
	VALUE v;
	a = &(self->attr[i]);
	switch (a->type) {
	    case -1:
		v = Qnil;
		break;
	    case FTInteger:
		v = INT2NUM(a->val.ival);
		break;
	    case FTDouble:
		v = rb_float_new(a->val.fval);
		break;
	    case FTString:
	    default:
		v = rb_str_new2(a->val.sval);
		break;
	}
	if (!NIL_P(v)) {
	    rb_hash_aset(h, rb_str_new2(a->name), v);
	}
    }
}

/* Shape#[] */
static VALUE sp_field_get(VALUE obj, VALUE name)
{
    shape_t *self;
    char *cname;
    int ofs;

    Data_Get_Struct(obj, shape_t, self);
    cname = StringValuePtr(name);
    ofs = shape_attrib_index(self, cname, 0);
    if (ofs < 0) {
	return Qnil;
    }
    switch (self->attr[ofs].type) {
	case -1:
	    return Qnil;
	case FTInteger:
	    return INT2NUM(self->attr[ofs].val.ival);
	case FTDouble:
	    return rb_float_new(self->attr[ofs].val.fval);
	case FTString:
	case FTLogical:
	    return rb_str_new2(self->attr[ofs].val.sval);
	default:
	    rb_raise(rb_eTypeError, "unacceptable field type");
    }
}

/* Shape#[]= */
static VALUE sp_field_set(VALUE obj, VALUE name, VALUE val)
{
    shape_t *self;
    char *cname;
    int	ofs;

    Data_Get_Struct(obj, shape_t, self);
    cname = StringValuePtr(name);
    ofs = shape_attrib_index(self, cname, 1);
    if (ofs < 0) {
	return Qnil;
    }
    switch (TYPE(val)) {
	case T_STRING:
	    self->attr[ofs].type = FTString;
	    self->attr[ofs].val.sval = StringDup(StringValuePtr(val));
	    break;
	case T_FIXNUM:
	    self->attr[ofs].type = FTInteger;
	    self->attr[ofs].val.ival = FIX2INT(val);
	    break;
	case T_FLOAT:
	    self->attr[ofs].type = FTDouble;
	    self->attr[ofs].val.fval = RFLOAT(val)->value;
	    break;
	case T_NIL:
	    self->attr[ofs].type = -1;
	    break;
	default:
	    rb_raise(rb_eTypeError,
		    "give me String, Integer, or Float");
    }
    return obj;
}

static VALUE sp_maxbound(VALUE obj)
{
    shape_t *self;
    double buf[4];
    Data_Get_Struct(obj, shape_t, self);
    buf[0] = self->obj->dfXMax;
    buf[1] = self->obj->dfYMax;
    buf[2] = self->obj->dfMMax;
    buf[3] = self->obj->dfZMax;
    return DoubleUnpackM(buf, 4, 2);
}

static VALUE sp_minbound(VALUE obj)
{
    shape_t *self;
    double buf[4];
    Data_Get_Struct(obj, shape_t, self);
    buf[0] = self->obj->dfXMin;
    buf[1] = self->obj->dfYMin;
    buf[2] = self->obj->dfMMin;
    buf[3] = self->obj->dfZMin;
    return DoubleUnpackM(buf, 4, 2);
}

static VALUE sp_mvals(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return DoubleUnpackM(self->obj->padfM, self->obj->nVertices, -1);
}

static VALUE sp_n_parts(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return INT2NUM(self->obj->nParts);
}

static VALUE sp_n_vertices(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return INT2NUM(self->obj->nVertices);
}

static VALUE sp_part_start(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    if (self->obj->nParts == 0)
	return Qnil;
    return IntUnpack(self->obj->panPartStart, self->obj->nParts);
}

static VALUE sp_part_type(VALUE obj)
{
    shape_t *self;
    VALUE ary;
    int i;
    Data_Get_Struct(obj, shape_t, self);
    if (self->obj->nParts == 0)
	return Qnil;
#if 0
    return UintUnpack(self->obj->panPartType, self->obj->nParts);
#endif
    ary = rb_ary_new2(self->obj->nParts);
    for (i = 0; i < self->obj->nParts; i++) {
	rb_ary_push(ary, PartType2Ruby(self->obj->panPartType[i]));
    }
    return ary;
}

static VALUE sp_shape_id(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return (self->obj->nShapeId < 0) ? Qnil : INT2NUM(self->obj->nShapeId);
}

static VALUE sp_shape_type(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return ShapeType2Ruby(self->obj->nSHPType);
}

static VALUE sp_rewind(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return SHPRewindObject(NULL, self->obj) ? Qtrue : Qfalse;
}

static VALUE sp_xvals(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return DoubleUnpack(self->obj->padfX, self->obj->nVertices);
}

static VALUE sp_yvals(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return DoubleUnpack(self->obj->padfY, self->obj->nVertices);
}

static VALUE sp_zvals(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    return DoubleUnpack(self->obj->padfZ, self->obj->nVertices);
}

/*
 * methods of Point/PointM/PointZ
 */

static VALUE pt_x(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    switch (self->obj->nSHPType) {
	case SHPT_POINT:
	case SHPT_POINTM:
	case SHPT_POINTZ:
	    return rb_float_new(self->obj->padfX[0]);
	default:
	    rb_bug("shape type %d is not Point", self->obj->nSHPType);
    }
    return Qnil;
}

static VALUE pt_y(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    switch (self->obj->nSHPType) {
	case SHPT_POINT:
	case SHPT_POINTM:
	case SHPT_POINTZ:
	    return rb_float_new(self->obj->padfY[0]);
	default:
	    rb_bug("shape type %d is not Point", self->obj->nSHPType);
    }
    return Qnil;
}

static VALUE pt_m(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    switch (self->obj->nSHPType) {
	case SHPT_POINTM:
	case SHPT_POINTZ:
	    return SHP_FLOAT_NEW(self->obj->padfM[0]);
	case SHPT_POINT:
	    return Qnil;
	default:
	    rb_bug("shape type %d is not Point", self->obj->nSHPType);
    }
    return Qnil;
}

static VALUE pt_z(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    switch (self->obj->nSHPType) {
	case SHPT_POINTZ:
	    return rb_float_new(self->obj->padfM[0]);
	case SHPT_POINTM:
	case SHPT_POINT:
	    return Qnil;
	default:
	    rb_bug("shape type %d is not Point", self->obj->nSHPType);
    }
    return Qnil;
}

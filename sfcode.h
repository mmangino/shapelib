/* sfcode.h */
/*
 * see bottom of the file for constructor
 */

static VALUE sf_add_field(int argc, VALUE *args, VALUE obj)
{
    shapefile_t *self;
    VALUE name, field_type, field_width, decimal_place;
    int r, atype, width, wdeci;

    rb_scan_args(argc, args, "31",
	    &name, &field_type, &field_width, &decimal_place);
    SECURE(name);
    StringValue(name);
    atype = Ruby2FieldType(field_type);
    width = NUM2INT(field_width);
    wdeci = NIL_P(decimal_place) ? 0 : NUM2INT(decimal_place);
    if (atype == FTDouble && argc == 3) {
	rb_raise(rb_eArgError, "specify decimal places for Float attribute");
    }
    Data_Get_Struct(obj, shapefile_t, self);
    r = shapefile_add_field(self, RSTRING(name)->ptr, atype, width, wdeci);
    return obj;
}

static VALUE sf_close(VALUE obj)
{
    shapefile_t *self;
    Data_Get_Struct(obj, shapefile_t, self);
    return INT2FIX(shapefile_close(self));
}

static VALUE sf_field_count(VALUE obj)
{
    shapefile_t *self;
    int r;
    Data_Get_Struct(obj, shapefile_t, self);
    r = shapefile_field_count(self);
    return INT2FIX(r);
}

static int Ruby2FieldIndex(shapefile_t *sfile, VALUE field)
{
    if (FIXNUM_P(field)) {
	return FIX2INT(field);
    } else {
	SECURE(field);
	return shapefile_field_index(sfile, StringValuePtr(field));
    }
}

static VALUE sf_delete_shape(VALUE obj, VALUE ishape)
{
    shapefile_t *self;
    Data_Get_Struct(obj, shapefile_t, self);
    shapefile_delete_shape(self, NUM2INT(ishape));
    return Qnil;
}

static VALUE sf_field_index(VALUE obj, VALUE name)
{
    shapefile_t *self;
    int ifield;
    Data_Get_Struct(obj, shapefile_t, self);
    ifield = Ruby2FieldIndex(self, name);
    return (ifield < 0) ? Qnil : INT2FIX(ifield);
}

static VALUE sf_field_name(VALUE obj, VALUE index)
{
    shapefile_t *self;
    const char *name;
    Data_Get_Struct(obj, shapefile_t, self);
    name = shapefile_field_name(self, NUM2INT(index));
    return name ? rb_str_new2(name) : Qnil;
}

static VALUE sf_field_type(VALUE obj, VALUE field)
{
    shapefile_t *self;
    int ifield;
    Data_Get_Struct(obj, shapefile_t, self);
    ifield = Ruby2FieldIndex(self, field);
    return FieldType2Ruby(shapefile_field_type(self, ifield));
}

static VALUE sf_field_width(VALUE obj, VALUE field)
{
    shapefile_t *self;
    int ifield;
    Data_Get_Struct(obj, shapefile_t, self);
    ifield = Ruby2FieldIndex(self, field);
    return INT2FIX(shapefile_field_width(self, ifield));
}

static VALUE sf_field_decimals(VALUE obj, VALUE field)
{
    shapefile_t *self;
    int ifield;
    Data_Get_Struct(obj, shapefile_t, self);
    ifield = Ruby2FieldIndex(self, field);
    return INT2FIX(shapefile_field_decimals(self, ifield));
}

static VALUE sf_fields(VALUE obj)
{
    shapefile_t *self;
    VALUE r;
    int fc, i;
    Data_Get_Struct(obj, shapefile_t, self);
    fc = shapefile_field_count(self);
    if (fc < 0)
	return Qnil;
    r = rb_ary_new2(fc);
    for (i = 0; i < fc; i++) {
	const char *fieldname;
	fieldname = shapefile_field_name(self, i);
	rb_ary_push(r, rb_str_new2(fieldname));
    }
    return r;
}

static VALUE sf_maxbound(VALUE obj)
{
    shapefile_t *self;
    double bound[4];
    double z;
    Data_Get_Struct(obj, shapefile_t, self);
    shapefile_bound(self, NULL, bound);
    z = bound[2];
    bound[2] = bound[3];
    bound[3] = z;
    return DoubleUnpack(bound, 4);
}

static VALUE sf_minbound(VALUE obj)
{
    shapefile_t *self;
    double bound[4];
    double z;
    Data_Get_Struct(obj, shapefile_t, self);
    shapefile_bound(self, bound, NULL);
    z = bound[2];
    bound[2] = bound[3];
    bound[3] = z;
    return DoubleUnpack(bound, 4);
}

static VALUE sf_read(int argc, VALUE *args, VALUE obj)
{
    shapefile_t *self;
    shape_t *s;
    VALUE ishape, r;
    int irec;
    rb_scan_args(argc, args, "01", &ishape);
    Data_Get_Struct(obj, shapefile_t, self);
    irec = NIL_P(ishape) ? -1 : NUM2INT(ishape);
    s = shapefile_read(self, irec);
    if (s == NULL)
	return Qnil;
    r = Data_Wrap_Struct(cShape, 0, shape_close, s);
    return r;
}

static VALUE sf_rewind(VALUE obj)
{
    shapefile_t *self;
    Data_Get_Struct(obj, shapefile_t, self);
    self->irec = 0;
    return obj;
}

static VALUE sf_each(VALUE obj)
{
    VALUE row;
    sf_rewind(obj);
    while (1) {
	row = sf_read(0, NULL, obj);
	if (NIL_P(row))
	    break;
	rb_yield(row);
    }
    return Qnil;
}

static VALUE sf_shape_type(VALUE obj)
{
    shapefile_t *self;
    Data_Get_Struct(obj, shapefile_t, self);
    return ShapeType2Ruby(shapefile_shape_type(self));
}

static VALUE sf_size(VALUE obj)
{
    shapefile_t *self;
    int r;
    Data_Get_Struct(obj, shapefile_t, self);
    r = shapefile_size(self);
    if (r < 0)
	rb_raise(rb_eRuntimeError, "size %d", r);
    return INT2FIX(r);
}

static VALUE sf_write(int argc, VALUE *args, VALUE obj)
{
    shape_t	*shape;
    shapefile_t *self;
    int		r, irec;
    if (argc < 1 || argc > 2) {
	rb_raise(rb_eArgError, "usage: Shapefile#write shape, [irec = -1]");
    }
    Data_Get_Struct(obj, shapefile_t, self);
    Data_Get_Struct(args[0], shape_t, shape);
    irec = ((argc == 2) ? NUM2INT(args[1]) : -1);
    r = shapefile_write(self, irec, shape);
    return INT2FIX(r);
}

/*
 * constructor
 */

static VALUE YieldIfPossible(VALUE proc, VALUE sf)
{
    if (!NIL_P(proc)) {
	rb_funcall(proc, rb_intern("call"), 1, sf);
	sf_close(sf);
	return Qnil;
    } else {
	return sf;
    }
}

static VALUE sf_s_open(int argc, VALUE *argv, VALUE klass)
{
    VALUE fnam, mode, sf, proc;
    shapefile_t *sfile;
    rb_scan_args(argc, argv, "11&", &fnam, &mode, &proc);
    SECURE(fnam);
    sfile = shapefile_open(StringValuePtr(fnam), 
	(NIL_P(mode) ? "rb" : StringValuePtr(mode))
	);
    sf = Data_Wrap_Struct(klass, 0, shapefile_close, sfile);
    return YieldIfPossible(proc, sf);
}

static VALUE sf_s_new(int argc, VALUE *argv, VALUE klass)
{
    VALUE fnam, shapetype, attrs, sf, proc;
    shapefile_t *sfile;
    int stype;
    int i;
    if (argc > 3 || argc < 2) {
	rb_raise(rb_eArgError,
	"usage: ShapeLib::Shapefile.new filename, shapetype, attrs = []");
	return Qnil;
    }
    rb_scan_args(argc, argv, "21&", &fnam, &shapetype, &attrs, &proc);
    SECURE(fnam);
    if ((stype = Ruby2ShapeType(shapetype)) == -1) {
	rb_raise(rb_eArgError, "shapetype must be Shape subclass");
    }
    if (argc > 2) {
	if (T_ARRAY != TYPE(attrs)) {
	    rb_raise(rb_eArgError, "attrs must be Array");
	}
    }
    sfile = shapefile_new(StringValuePtr(fnam), stype);
    if (sfile == NULL) {
	rb_raise(rb_eRuntimeError, "shapefile_new failed");
    }
    sf = Data_Wrap_Struct(klass, 0, shapefile_close, sfile);
    if (argc > 2) {
	for (i = 0; i < RARRAY(attrs)->len; i++) {
	    VALUE p = RARRAY(attrs)->ptr[i];
	    if (T_ARRAY != TYPE(p)) {
		rb_raise(rb_eArgError, "non-array content in attrs");
	    }
	    sf_add_field(RARRAY(p)->len, RARRAY(p)->ptr, sf);
	}
    }
    return YieldIfPossible(proc, sf);
}

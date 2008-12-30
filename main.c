#include <ruby.h>
#if defined(HAVE_SHAPEFIL_H)
	#include <shapefil.h>
#endif
#if defined(HAVE_LIBSHP_SHAPEFIL_H)
	#include <libshp/shapefil.h>
#endif
#include "shpplus.h"

typedef struct {
    const char *mname;
    VALUE (*func)();
    int argc;
} method_table_entry;

typedef struct {
    const char *name;
    VALUE val;
} const_table_entry;

static VALUE mShapeLib;
static VALUE cShapeFile;
static VALUE cShape;
static VALUE cPoint;
static VALUE cPointM;
static VALUE cPointZ;
static VALUE cArc;
static VALUE cArcM;
static VALUE cArcZ;
static VALUE cPolygon;
static VALUE cPolygonM;
static VALUE cPolygonZ;
static VALUE cMultiPoint;
static VALUE cMultiPointM;
static VALUE cMultiPointZ;
static VALUE cMultiPatch;

/* code begins here */

#include "valconv.h"
#include "sfcode.h"
#include "sflist.h"
#include "spcode.h"
#include "spwkt.h"
#include "splist.h"

static void register_consts(VALUE klass, const_table_entry *table)
{
    const_table_entry *p;
    for (p = table; p->name; p++) {
	rb_define_const(klass, p->name, p->val);
    }
}

static void register_methods(VALUE klass, method_table_entry *table)
{
    method_table_entry *p;
    for (p = table; p->mname; p++) {
	rb_define_method(klass, p->mname, p->func, p->argc);
    }
}

void Init_shapelib()
{
    mShapeLib = rb_define_module("ShapeLib");
    cShapeFile = rb_define_class_under(mShapeLib, "ShapeFile", rb_cObject);
    cShape = rb_define_class_under(mShapeLib, "Shape", rb_cObject);
    cPoint = rb_define_class_under(mShapeLib, "Point", cShape);
    cPointM = rb_define_class_under(mShapeLib, "PointM", cPoint);
    cPointZ = rb_define_class_under(mShapeLib, "PointZ", cPointM);
    cArc = rb_define_class_under(mShapeLib, "Arc", cShape);
    cArcM = rb_define_class_under(mShapeLib, "ArcM", cArc);
    cArcZ = rb_define_class_under(mShapeLib, "ArcZ", cArcM);
    cPolygon = rb_define_class_under(mShapeLib, "Polygon", cShape);
    cPolygonM = rb_define_class_under(mShapeLib, "PolygonM", cPolygon);
    cPolygonZ = rb_define_class_under(mShapeLib, "PolygonZ", cPolygonM);
    cMultiPoint = rb_define_class_under(mShapeLib, "MultiPoint", cShape);
    cMultiPointM = rb_define_class_under(mShapeLib, "MultiPointM", cMultiPoint);
    cMultiPointZ = rb_define_class_under(mShapeLib, "MultiPointZ", cMultiPointM);
    cMultiPatch = rb_define_class_under(mShapeLib, "MultiPatch", cShape);
    rb_define_module_function(mShapeLib, "new_point", sl_m_new_point, -1);
    rb_define_singleton_method(cShapeFile, "new", sf_s_new, -1);
    rb_define_singleton_method(cShapeFile, "open", sf_s_open, -1);
    rb_define_singleton_method(cShape, "new", sp_s_new, 1);
    rb_define_singleton_method(cPoint, "new", pt_s_new, -1);
    rb_define_singleton_method(cPointM, "new", ptm_s_new, -1);
    rb_define_singleton_method(cPointZ, "new", ptz_s_new, -1);
    rb_define_singleton_method(cArc, "new", arc_s_new, -1);
    rb_define_singleton_method(cArcM, "new", arcm_s_new, -1);
    rb_define_singleton_method(cArcZ, "new", arcz_s_new, -1);
    rb_define_singleton_method(cPolygon, "new", pl_s_new, -1);
    rb_define_singleton_method(cPolygonM, "new", plm_s_new, -1);
    rb_define_singleton_method(cPolygonZ, "new", plz_s_new, -1);
    rb_define_singleton_method(cMultiPoint, "new", mp_s_new, -1);
    rb_define_singleton_method(cMultiPointM, "new", mpm_s_new, -1);
    rb_define_singleton_method(cMultiPointZ, "new", mpz_s_new, -1);
    rb_define_singleton_method(cMultiPatch, "new", mpatch_s_new, -1);
    register_consts(mShapeLib, sl_consts);
    register_methods(cShapeFile, sf_methods);
    register_methods(cShape, sp_methods);
    register_methods(cPoint, pt_methods);
}

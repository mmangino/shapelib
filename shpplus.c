#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#if defined(HAVE_SHAPEFIL_H)
	#include <shapefil.h>
#endif
#if defined(HAVE_LIBSHP_SHAPEFIL_H)
	#include <libshp/shapefil.h>
#endif
#include <ruby.h>
#include "shpplus.h"

#define HAS_DBFMARKRECORDDELETED 0
#define ATTR_TABLE_DEFAULT	16

/*
 * === static functions ===
 */

static shape_t *shape_init(SHPObject *objp);

/*--- lower level routines ---*/

static int FieldNameEquiv(const char *a, const char *b)
{
    int i;
    monitor_someday(("FNE <%s> <%s>", a, b));
    for (i = 0; i < 11; i++) {
	int ca, cb;
	if (a[i] == '\0' && b[i] == '\0') {
	    return 1;
	} else if (a[i] == '\0' || b[i] == '\0') {
	    return 0;
	}
	ca = (isalpha(a[i]) ? toupper(a[i]) : (a[i]));
	cb = (isalpha(b[i]) ? toupper(b[i]) : (b[i]));
	if (ca != cb) {
	    return 0;
	}
    }
    return 1;
}

static void AttribClear(attrib_t *pa)
{
    pa->type = -1;
    pa->val.fval = 0.0;
    memset(pa->name, '\0', DBF_NAMELEN);
}

/*--- related to struct shape_t ---*/

static int ShapeSetAttrTable(shape_t *sh, int newsize)
{
    int	oldsize;
    int i;
    if (newsize == 0) {
	if (sh->nattr != 0)
	    rb_bug("trying to emptify attr table with %d entries", sh->nattr);
	return -1;
    }
    if (sh->attr == NULL) {
	sh->attr = xmalloc(sizeof(attrib_t) * newsize);
    } else {
	sh->attr = xrealloc(sh->attr, sizeof(attrib_t) * newsize);
    }
    oldsize = sh->nattr;
    sh->nattr = newsize;
    for (i = oldsize; i < newsize; i++) {
	AttribClear(&(sh->attr[i]));
    }
    return 0;
}

/*--- related to struct shapefile_t ---*/

static shapefile_t *ShapefileInit(void)
{
    shapefile_t *sfile;
    sfile = xmalloc(sizeof(shapefile_t));
    if (sfile == NULL)
	return NULL;
    sfile->nrecs = sfile->nfields = -1;
    sfile->ftab = NULL;
    sfile->irec = 0;
    return sfile;
}

static void ShapefileBuildFieldTable(shapefile_t *sfile)
{
    int i;
    if (sfile->ftab)
	return;
    sfile->nfields = shapefile_field_count(sfile);
    sfile->ftab = xmalloc(sizeof(field_t) * sfile->nfields);
    if (sfile->ftab == NULL)
	return;
    for (i = 0; i < sfile->nfields; i++) {
	memset(sfile->ftab[i].name, '\0', DBF_NAMELEN);
    }
    for (i = 0; i < sfile->nfields; i++) {
	field_t		*f;
	f = &(sfile->ftab[i]);
	f->type = DBFGetFieldInfo(sfile->hdbf, i, f->name, NULL, NULL);
	monitor(("DBFGetFieldInfo(%p, %d) => %d, \"%s\"", sfile->hdbf, i,
	    (int)f->type, f->name));
    }
}

static void ShapefileWriteAttribute(shapefile_t *sfile,
    int irec, int ifield, attrib_t *a)
{
    int s;
    if (a->type == -1) {
	rb_bug("supposed to be rejected in shapefile_write");
    } else if (a->type == FTInteger) {
	s = DBFWriteIntegerAttribute(sfile->hdbf, irec, ifield,
	    a->val.ival);
	monitor(("DBFWriteIntegerAttribute(%p %d %d %d) -> %d",
	    sfile->hdbf, irec, ifield, a->val.ival, s));
    } else if (a->type == FTDouble) {
	s = DBFWriteDoubleAttribute(sfile->hdbf, irec, ifield,
	    a->val.fval);
	monitor(("DBFWriteDoubleAttribute(%p %d %d %g) -> %d",
		sfile->hdbf, irec, ifield, a->val.fval, s));
    } else {
	s = DBFWriteStringAttribute(sfile->hdbf, irec, ifield,
	    a->val.sval);
	monitor(("DBFWriteStringAttribute(%p %d %d %s) -> %d",
	    sfile->hdbf, irec, ifield, a->val.sval, s));
    }
    if (s == 0) {
	rb_raise(rb_eRuntimeError, "write error or overflow");
    }
}

/*
 * === methods of class ShapeFile ===
 */

shapefile_t *shapefile_open(const char *filename, const char *mode)
{
    shapefile_t *sfile;
    sfile = ShapefileInit();
    sfile->hshp = SHPOpen(filename, mode);
    monitor(("SHPOpen(\"%s\", \"%s\") -> %p", filename, mode, sfile->hshp));
    sfile->hdbf = DBFOpen(filename, mode);
    monitor(("DBFOpen(\"%s\", \"%s\") -> %p", filename, mode, sfile->hdbf));
    ShapefileBuildFieldTable(sfile);
    return sfile;
}

shapefile_t *shapefile_new(const char *filename, int shptype)
{
    shapefile_t *sfile;
    sfile = ShapefileInit();
    sfile->hshp = SHPCreate(filename, shptype);
    monitor(("SHPCreate(\"%s\", %d) -> %p", filename, shptype, sfile->hshp));
    sfile->hdbf = DBFCreate(filename);
    monitor(("DBFCreate(\"%s\") -> %p", filename, sfile->hdbf));
    return sfile;
}

int shapefile_add_field(shapefile_t *sfile, const char *name,
    int atype, int width, int decimals)
{
    int r;
    if (sfile->hdbf == NULL) {
	rb_raise(rb_eRuntimeError, "dbf closed");
	return -1;
    }
    if (sfile->ftab) {
	rb_raise(rb_eRuntimeError, "add field before you write a record");
	return -1;
    }
    monitor(("DBFAddField(%p, %s, %d, %d, %d)", sfile->hdbf, name,
	atype, width, decimals));
    r = DBFAddField(sfile->hdbf, name, (DBFFieldType)atype, width, decimals);
    if (r == -1) {
	rb_raise(rb_eRuntimeError, "DBFAddField fail");
    }
    return r;
}

int shapefile_close(shapefile_t *sfile)
{
    if (sfile->hshp) {
	monitor(("SHPClose(%p)", sfile->hshp));
    	SHPClose(sfile->hshp);
	sfile->hshp = NULL;
    }
    if (sfile->hdbf) {
	monitor(("DBFClose(%p)", sfile->hdbf));
	DBFClose(sfile->hdbf);
	sfile->hdbf = NULL;
    }
    if (sfile->ftab) {
	xfree(sfile->ftab);
	sfile->ftab = NULL;
    }
    return 0;
}

int shapefile_field_decimals(shapefile_t *sfile, unsigned ifield)
{
    int decimals;
    if (NULL == sfile->hdbf)
	return -1;
    DBFGetFieldInfo(sfile->hdbf, ifield, NULL, NULL, &decimals);
    return decimals;
}

int shapefile_delete_shape(shapefile_t *sfile, int ishape)
{
#if HAS_DBFMARKRECORDDELETED
    int r;
    if (ishape < 0) {
	rb_raise(rb_eArgError, "shape number %d must be nonnegative", ishape);
    }
    if (sfile->hshp) {
	SHPObject *shape;
	double d;
	shape = SHPCreateSimpleObject(SHPT_NULL, 0, &d, &d, &d);
	if (shape == NULL) {
	    rb_raise(rb_eRuntimeError,
		"SHPCreateSimpleObject(SHPT_NULL, ...)");
	}
	r = SHPWriteObject(sfile->hshp, ishape, shape);
	if (r != ishape) {
	    rb_raise(rb_eRuntimeError,
		"SHPWriteObject(NullObj, %d) -> %d", ishape, r);
	}
	SHPDestroyObject(shape);
    }
    if (sfile->hdbf) {
	r = DBFMarkRecordDeleted(sfile->hdbf, ishape, 1);
	if (r) {
	    rb_raise(rb_eRuntimeError,
		"DBFMarkRecordDeleted %d -> %d", ishape, r);
	}
    }
#else
    rb_raise(rb_eRuntimeError, "DBFMarkRecordDeleted is disabled");
#endif
    return 0;
}

int shapefile_field_index(shapefile_t *sfile, const char *name)
{
    if (NULL == sfile->hdbf)
	return -1;
    return DBFGetFieldIndex(sfile->hdbf, name);
}

/* WARNING: result of this function is not valid after next call.
 */
const char *shapefile_field_name(shapefile_t *sfile, unsigned ifield)
{
    static char buf[DBF_NAMELEN];
    if (NULL == sfile->hdbf)
	return NULL;
    DBFGetFieldInfo(sfile->hdbf, ifield, buf, NULL, NULL);
    return buf;
}

DBFFieldType shapefile_field_type(shapefile_t *sfile, unsigned ifield)
{
    if (NULL == sfile->hdbf) {
	return -1;
    }
    if (sfile->ftab) {
	if (ifield >= sfile->nfields) {
	    return -1;
	}
	return sfile->ftab[ifield].type;
    }
    return DBFGetFieldInfo(sfile->hdbf, ifield, NULL, NULL, NULL);
}

int shapefile_field_width(shapefile_t *sfile, unsigned ifield)
{
    int width;
    if (NULL == sfile->hdbf)
	return -1;
    DBFGetFieldInfo(sfile->hdbf, ifield, NULL, &width, NULL);
    return width;
}

int shapefile_field_count(shapefile_t *sfile)
{
    int	r;
    if (NULL == sfile->hdbf) {
	return -1;
    }
    if (sfile->ftab) {
	return sfile->nfields;
    }
    /*
     * Field table is not built here
     * because it would prohibit field_add() afterwards.
     */
    r = DBFGetFieldCount(sfile->hdbf);
    monitor(("DBFGetFieldCount(%p) -> %d", sfile->hdbf, r));
    return r;
}

void shapefile_bound(shapefile_t *sfile, double *minbound, double *maxbound)
{
    if (NULL == sfile->hshp) {
	int i;
	/*
	 * is that right? I guess NaN should be filled - if there is
	 * a portable and safe way to generate it.
	 */
	if (minbound) { for (i = 0; i < 4; i++) minbound[i] = 0.0; }
	if (maxbound) { for (i = 0; i < 4; i++) maxbound[i] = 0.0; }
	return;
    }
    SHPGetInfo(sfile->hshp, NULL, NULL, minbound, maxbound);
}

shape_t *shapefile_read(shapefile_t *sfile, int ishape)
{
    shape_t *shape;
    int i, iattr;
    if (NULL == sfile->hshp)
	return NULL;
    if (ishape < 0) {
	if (sfile->nrecs < 0) {
	    shapefile_size(sfile);
	    sfile->irec = 0;
	}
	ishape = (sfile->irec)++;
    } else {
	sfile->irec = ishape + 1;
    }
    shape = shape_init(SHPReadObject(sfile->hshp, ishape));
    if (shape == NULL)
	return NULL;
    monitor(("SHPReadObject(%p) -> %p", sfile->hshp, shape));
    if (shape->obj->nSHPType == SHPT_NULL) {
	monitor(("shape type NULL"));
	return NULL;
    }
    ShapefileBuildFieldTable(sfile);
    if (NULL == sfile->hdbf)
	goto no_dbf;
#if HAS_DBFMARKRECORDDELETED
    if (DBFIsRecordDeleted(sfile->hdbf, ishape)) {
	monitor(("DBF record %d deleted", ishape))
	goto no_dbf;
    }
#endif
    ShapeSetAttrTable(shape, sfile->nfields);
    for (i = iattr = 0; i < sfile->nfields; i++) {
	attrib_t *a;
	field_t	*f;
	if (DBFIsAttributeNULL(sfile->hdbf, ishape, i)) {
	    goto next_field;
	}
	a = &(shape->attr[iattr]);
	f = &(sfile->ftab[i]);
	monitor(("attrib %s type %d", f->name, f->type));
	if (f->type == FTInteger) {
	    a->val.ival = DBFReadIntegerAttribute(sfile->hdbf, ishape, i);
	} else if (f->type == FTDouble) {
	    a->val.fval = DBFReadDoubleAttribute(sfile->hdbf, ishape, i);
#if 0
	} else if (f->type == FTLogical) {
	    a->val.ival = DBFReadLogicalAttribute(sfile->hdbf, ishape, i);
#endif
	} else {
	    a->val.sval = StringDup(
		DBFReadStringAttribute(sfile->hdbf, ishape, i)
		);
	}
	a->type = f->type;
	memcpy(a->name, f->name, DBF_NAMELEN);
	iattr++;
    next_field:
	;
    }
no_dbf:
    return shape;
}

int shapefile_shape_type(shapefile_t *sfile)
{
    int r;
    if (NULL == sfile->hshp)
	return -1;
    SHPGetInfo(sfile->hshp, NULL, &r, NULL, NULL);
    return r;
}

int shapefile_size(shapefile_t *sfile)
{
    int sd, ss;
    if (NULL == sfile->hshp) {
	return -1;
    }
    SHPGetInfo(sfile->hshp, &ss, NULL, NULL, NULL);
    monitor(("SHPGetInfo(%p) -> %d", sfile->hshp, ss));
    if (sfile->hdbf) {
	sd = DBFGetRecordCount(sfile->hdbf);
	monitor(("DBFGetRecordCount(%p) -> %d", sfile->hdbf, sd));
	if (sd != ss) {
	    if (DBFGetFieldCount(sfile->hdbf) > 0) {
		rb_warn("ShapeFile/DBF size mismatch %d %d", ss, sd);
	    }
	}
    }
    sfile->nrecs = ss;
    return ss;
}

int shapefile_write(shapefile_t *sfile, int irec, shape_t *shape)
{
    int r;
    int ifield;
    int iattr;
    int *usedfield;
    /* precondition */
    if (!sfile->hshp || !shape->obj)
	return -1;
    ShapefileBuildFieldTable(sfile);
    /* write the object */
    r = SHPWriteObject(sfile->hshp, irec, shape->obj);
    monitor(("SHPWriteObject(%p, %d, %p) -> %d", sfile->hshp, irec,
    	shape->obj, r));
    if (r < 0)
	return -1;
    /* make field table */
    usedfield = xmalloc(sizeof(int) * sfile->nfields);
    for (ifield = 0; ifield < sfile->nfields; ifield++) {
	usedfield[ifield] = 0;
    }
    for (iattr = 0; iattr < shape->nattr; iattr++) {
	attrib_t	*a;
	a = &(shape->attr[iattr]);
	if (a->type == -1) {
	    continue;
	}
	ifield = DBFGetFieldIndex(sfile->hdbf, a->name);
	if (ifield == -1) {
	    rb_raise(rb_eArgError,
		"attribute %s not defined in ShapeFile", a->name);
	}
	ShapefileWriteAttribute(sfile, r, ifield, a);
	usedfield[ifield] = 1;
    }
    for (ifield = 0; ifield < sfile->nfields; ifield++) {
	int r2;
	if (usedfield[ifield] == 0) {
	    r2 = DBFWriteNULLAttribute(sfile->hdbf, r, ifield);
	    monitor(("DBFWriteNULLAttribute(%p %d %d -> %d",
		sfile->hdbf, r, ifield, r2));
	    if (r2 == 0)
		rb_raise(rb_eRuntimeError, "write error");
	}
    }
    free(usedfield);
    return r;
}

static shape_t *shape_init(SHPObject *objp)
{
    shape_t *sh;
    if (objp == NULL)
	return NULL;
    sh = xmalloc(sizeof(shape_t));
    if (sh == NULL) {
	SHPDestroyObject(objp);
	return NULL;
    }
    sh->obj = objp;
    sh->attr = NULL;
    sh->nattr = 0;
    return sh;
}

shape_t *shape_new(int shape_type, int shape_id,
	int n_parts, int *part_type, int *part_start, int n_vertices,
	double *xvals, double *yvals, double *zvals, double *mvals)
{
    SHPObject *objp;
    shape_t *sh;
    objp = SHPCreateObject(shape_type, shape_id,
		n_parts, part_start, part_type, n_vertices,
		xvals, yvals, zvals, mvals);
    monitor(("SHPCreateObject(%d, %d, ...) -> %p",
		shape_type, shape_id, objp));
    sh = shape_init(objp);
    return sh;
}

shape_t *shape_new_point(int shapetype, double x, double y, double m, double z)
{
    shape_t *sh;
    if (m == 0.0) {
	sh = shape_init(SHPCreateSimpleObject(shapetype, 1, &x, &y, &z));
	monitor(("SHPCreateSimpleObject(%d, 1 %g, %g, %g) -> %p",
	    shapetype, x, y, z, sh));
    } else {
	sh = shape_init(SHPCreateObject(shapetype,
	    -1, /* iShape: dummy because shapefile_write determines it */
	    0, /* nParts: 0 indicates panPartStart is not given */
	    NULL, /* panPartStart: list of part start address */
	    NULL, /* panPartType: list of MultiPatch part type */
	    1, /* nVertices */
	    &x, &y, &z, &m));
	monitor(("SHPCreateObject(%d, ..., 1 %g, %g, %g, %g) -> %p",
	    shapetype, x, y, m, z, sh));
    }
    return sh;
}

shape_t *shape_new_obj(int shapetype,
    int n_parts, int *part_start, int *part_type, int n_vert,
    double *x, double *y, double *m, double *z)
{
    shape_t *sh;
    sh = shape_init(SHPCreateObject(shapetype,
	-1, /* iShape: dummy because shapefile_write determines it */
	n_parts, part_start, part_type, n_vert, x, y, z, m));
    monitor(("SHPCreateObject(%d, -1, %d, ... %d, [%g, %g, %g, %g], ...) -> %p",
	shapetype, n_parts, n_vert, x, y, m, z, sh));
    return sh;
}

int shape_attrib_index(shape_t *sh, const char *key, int extend) 
{
    int i;
    monitor(("shape_attrib_index(%p, %s)", sh, key));
    if (sh->attr == NULL || sh->nattr == 0) {
	if (!extend) {
	    return -1;
	}
	ShapeSetAttrTable(sh, ATTR_TABLE_DEFAULT);
    }
    for (i = 0; i < sh->nattr; i++) {
	if (FieldNameEquiv(sh->attr[i].name, key)) {
	    return i;
	}
    }
    for (i = 0; i < sh->nattr; i++) {
	if (sh->attr[i].type == -1) {
	    strcpy(sh->attr[i].name, key);
	    return i;
	}
    }
    if (extend) {
	i = sh->nattr;
	ShapeSetAttrTable(sh, sh->nattr * 2);
	strcpy(sh->attr[i].name, key);
	return i;
    } else {
	return -1;
    }
}

int shape_close(shape_t *shape)
{
    if (shape->obj) {
	monitor(("SHPDestroyObject(%p)", shape->obj));
	SHPDestroyObject(shape->obj);
	shape->obj = NULL;
    }
    if (shape->attr) {
	int i;
	for (i = 0; i < shape->nattr; i++) {
	    switch (shape->attr[i].type) {
		case FTInteger:
		case FTDouble:
		    break;
		case FTString:
		default:
		    free(shape->attr[i].val.sval);
		    shape->attr[i].val.ival = 0;
		    break;
	    }
	}
	free(shape->attr);
	shape->attr = NULL;
    }
    xfree(shape);
    return 0;
}

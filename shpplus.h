/* requires <shapefil.h> */
#define MONITOR_DETAIL 0

#define DBF_NAMELEN 12

typedef struct field_t {
    DBFFieldType	type;
    char		name[DBF_NAMELEN];
} field_t;

typedef struct {
    SHPHandle	hshp;
    DBFHandle	hdbf;
    int		nfields;
    field_t	*ftab;
    /* for iterator */
    int		nrecs;
    int		irec;
} shapefile_t;

typedef struct {
    DBFFieldType	type;
    char	name[DBF_NAMELEN];
    union ifs_t {
	int		ival;
	double	fval;
	char	*sval;
    }	val;
} attrib_t;

typedef struct {
    SHPObject	*obj;
    int		nattr;
    attrib_t	*attr;
} shape_t;

extern shapefile_t *shapefile_new(const char *filename, int shptype);
extern shapefile_t *shapefile_open(const char *filename, const char *mode);
extern int shapefile_add_field(shapefile_t *sfile, const char *name,
    int atype, int width, int decimals);
extern void shapefile_bound(shapefile_t *sfile, double *minbound,
	double *maxbound);
extern int shapefile_close(shapefile_t *sfile);
extern int shapefile_field_count(shapefile_t *sfile);
extern int shapefile_field_decimals(shapefile_t *sfile, unsigned ifield);
extern int shapefile_delete_shape(shapefile_t *sfile, int ishape);
extern int shapefile_field_index(shapefile_t *sfile, const char *name);
extern const char *shapefile_field_name(shapefile_t *sfile, unsigned ifield);
extern DBFFieldType shapefile_field_type(shapefile_t *sfile, unsigned ifield);
extern int shapefile_field_width(shapefile_t *sfile, unsigned ifield);
extern shape_t *shapefile_read(shapefile_t *sfile, int ishape);
extern int shapefile_size(shapefile_t *sfile);
extern int shapefile_shape_type(shapefile_t *sfile);
extern int shapefile_write(shapefile_t *sfile, int irec, shape_t *shape);

extern shape_t *shape_new(int shape_type, int shape_id,
	int n_parts, int *part_type, int *part_start, int n_vertices,
	double *xvals, double *yvals, double *zvals, double *mvals);
extern shape_t *shape_new_point(int shapetype,
	double x, double y, double m, double z);
extern shape_t *shape_new_obj(int shapetype,
	int n_parts, int *part_start, int *part_type, int n_vert,
	double *x, double *y, double *m, double *z);
extern int shape_close(shape_t *shape);
extern int shape_attrib_index(shape_t *sh, const char *key, int extend);

/*
 * MACRO AND MACRO-LIKE STATIC FUNCTION
 */

#if MONITOR_DETAIL
# define monitor(args) rb_warning args
#else
# define monitor(args)
#endif
#define monitor_someday(args)

static char *StringDup(const char *str)
{
    char *r;
    size_t n;
    n = strlen(str) + 1;
    r = xmalloc(n);
    memcpy(r, str, n);
    return r;
}

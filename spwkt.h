
/* safer substitute of strcat(3). I believe strcat is too dangerous.
 */
static char *StringCat(char *buf, char *src, char *stop)
{
    if (*buf)
	buf = buf + strlen(buf);
    strncpy(buf, src, stop - buf);
    *stop = '\0';
    return buf + strlen(buf);
}

/* 123..........45678 */
/* -0.1234567890e-100 */
static int wkt_dlen = 18;
static char wkt_dfmt[16] = "%1.10g";

static char *DblFmtCat(char *buf, double x, char *stop)
{
    char fbuf[512];
    sprintf(fbuf, wkt_dfmt, x);
    return StringCat(buf, fbuf, stop);
}

static VALUE pt_wkt(VALUE obj)
{
    shape_t *self;
    size_t buflen;
    char *buf, *cur, *stop;
    VALUE wkt;
    Data_Get_Struct(obj, shape_t, self);
    buflen = wkt_dlen * 2 + 9;
    cur = buf = xmalloc(buflen + 1);
    stop = buf + buflen;
    strcpy(cur, "POINT(");
    cur = DblFmtCat(cur, self->obj->padfX[0], stop);
    cur = StringCat(cur, " ", stop);
    cur = DblFmtCat(cur, self->obj->padfY[0], stop);
    cur = StringCat(cur, ")", stop);
    wkt = rb_str_new(buf, strlen(buf));
    free(buf);
    return wkt;
}

static VALUE mp_wkt(VALUE obj)
{
    shape_t *self;
    VALUE wkt;
    size_t buflen;
    char *buf, *cur, *stop;
    int iv;
    Data_Get_Struct(obj, shape_t, self);
    buflen = (2 * wkt_dlen + 3) * self->obj->nVertices + 11;
    cur = buf = xmalloc(buflen + 1);
    stop = buf + buflen;
    strcpy(buf, "MULTIPOINT(");
    for (iv = 0; iv < self->obj->nVertices; iv++) {
	if (iv)
	    StringCat(cur, ", ", stop);
	cur = DblFmtCat(cur, self->obj->padfX[iv], stop);
	cur = StringCat(cur, " ", stop);
	cur = DblFmtCat(cur, self->obj->padfY[iv], stop);
    }
    StringCat(cur, ")", stop);
    wkt = rb_str_new(buf, strlen(buf));
    free(buf);
    return wkt;
}

static VALUE WKTSinglePolygon(SHPObject *obj)
{
    size_t buflen;
    VALUE wkt;
    char *buf, *cur, *stop;
    int ipart, endvert, ivert;
    buflen = (2 * wkt_dlen + 3) * obj->nVertices + obj->nParts * 2 + 9;
    cur = buf = xmalloc(buflen + 1);
    stop = cur + buflen;
    strcpy(buf, "POLYGON(");
    for (ipart = 0; ipart < obj->nParts; ipart++) {
	if (ipart == obj->nParts - 1) {
	    endvert = obj->nVertices;
	} else {
	    endvert = obj->panPartStart[ipart + 1];
	}
	if (ipart)
	    cur = StringCat(cur, ", ", stop);
	cur = StringCat(cur, "(", stop);
	for (ivert = obj->panPartStart[ipart]; ivert < endvert; ivert++) {
	    if (ivert > obj->panPartStart[ipart])
		cur = StringCat(cur, ", ", stop);
	    cur = DblFmtCat(cur, obj->padfX[ivert], stop);
	    cur = StringCat(cur, " ", stop);
	    cur = DblFmtCat(cur, obj->padfY[ivert], stop);
	}
	cur = StringCat(cur, ")", stop);
    }
    cur = StringCat(cur, ")", stop);
    wkt = rb_str_new(buf, strlen(buf));
    free(buf);
    return wkt;
}

static VALUE pl_wkt(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    /* === BUG ===
     * following code doesn't consider the possibilities:
     * - there may be multiple outer loop in a Polygon Shape.
     *   in such a case, it must generate MULTIPOLYGON WKT.
     * - inner loop (hole) may come before outer loop.
     */
    return WKTSinglePolygon(self->obj); 
#if 0
    return WKTMultiPolygon(self->obj);
#endif
}

static VALUE WKTSingleArc(SHPObject *obj)
{
    size_t buflen;
    VALUE wkt;
    char *buf, *cur, *stop;
    int ivert;
    buflen = (2 * wkt_dlen + 3) * obj->nVertices + 10;
    cur = buf = xmalloc(buflen + 1);
    stop = cur + buflen;
    strcpy(buf, "LINESTRING(");
    for (ivert = 0; ivert < obj->nVertices; ivert++) {
	if (ivert)
	    cur = StringCat(cur, ", ", stop);
	cur = DblFmtCat(cur, obj->padfX[ivert], stop);
	cur = StringCat(cur, " ", stop);
	cur = DblFmtCat(cur, obj->padfY[ivert], stop);
    }
    cur = StringCat(cur, ")", stop);
    wkt = rb_str_new(buf, strlen(buf));
    free(buf);
    return wkt;
}

static VALUE WKTMultiArc(SHPObject *obj)
{
    size_t buflen;
    VALUE wkt;
    char *buf, *cur, *stop;
    int ipart, endvert, ivert;
    buflen = (2 * wkt_dlen + 3) * obj->nVertices + obj->nParts * 1 + 16;
    cur = buf = xmalloc(buflen + 1);
    stop = cur + buflen;
    strcpy(buf, "MULTILINESTRING(");
    for (ipart = 0; ipart < obj->nParts; ipart++) {
	if (ipart == obj->nParts - 1) {
	    endvert = obj->nVertices;
	} else {
	    endvert = obj->panPartStart[ipart + 1];
	}
	if (ipart)
	    cur = StringCat(cur, ", ", stop);
	cur = StringCat(cur, "(", stop);
	for (ivert = obj->panPartStart[ipart]; ivert < endvert; ivert++) {
	    if (ivert > obj->panPartStart[ipart])
		cur = StringCat(cur, ", ", stop);
	    cur = DblFmtCat(cur, obj->padfX[ivert], stop);
	    cur = StringCat(cur, " ", stop);
	    cur = DblFmtCat(cur, obj->padfY[ivert], stop);
	}
	cur = StringCat(cur, ")", stop);
    }
    cur = StringCat(cur, ")", stop);
    wkt = rb_str_new(buf, strlen(buf));
    free(buf);
    return wkt;
}

static VALUE arc_wkt(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    if (self->obj->nParts > 1) {
	return WKTMultiArc(self->obj);
    } else {
	return WKTSingleArc(self->obj);
    }
}

static VALUE sp_wkt(VALUE obj)
{
    shape_t *self;
    Data_Get_Struct(obj, shape_t, self);
    switch (self->obj->nSHPType) {
	case SHPT_POINT:
	case SHPT_POINTM:
	case SHPT_POINTZ:
	    return pt_wkt(obj);
	    break;
	case SHPT_ARC:
	case SHPT_ARCM:
	case SHPT_ARCZ:
	    return arc_wkt(obj);
	    break;
	case SHPT_POLYGON:
	case SHPT_POLYGONM:
	case SHPT_POLYGONZ:
	    return pl_wkt(obj);
	    break;
	case SHPT_MULTIPOINT:
	case SHPT_MULTIPOINTM:
	case SHPT_MULTIPOINTZ:
	    return mp_wkt(obj);
	    break;
	case SHPT_MULTIPATCH:
	    return pl_wkt(obj);
	    break;
    }
    rb_bug("invalid shape type %d", self->obj->nSHPType);
    return Qnil;
}

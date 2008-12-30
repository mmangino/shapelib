=begin
= module ShapeLib

This module is a wrapper of C Shapefile Library
(also called ((<ShapeLib|URL:http://shapelib.maptools.org/>))).
It consists of basically two classes:
((<ShapeFile|class ShapeFile>)) and ((<Shape|class Shape>)).
If you are unfamiliar with object-oriented programming,
please see ((<Appendix A: C to Ruby equivalence table>)).

A extension library (({shapelib})) must be required before using this module:

  require 'shapelib'

== module functions 
Following three functions construct a point object.
It can be used like top level function after (({include})):

  include ShapeLib
  pt = new_point(1, 2)

Arguments ((|x|)), ((|y|)), ((|z|)), and ((|m|)) ((<should be Float>)).

--- ShapeLib.new_point x, y
    creates a ((<class Point>)) object.
--- ShapeLib.new_point x, y, m
    creates a ((<class PointM>)) object.
--- ShapeLib.new_point x, y, m, z
    creates a ((<class PointZ>)) object.

== class ShapeFile
=== class methods
Following two class methods (({new})) and (({open}))
return an instance of ((<class ShapeFile>)) if succeeded.
Exception is raised on failure.
It is recommended to call ((<shapefile.close>))
when each shapefile is no longer used.

--- ShapeFile::open filename, access = 'rb'
--- ShapeFile::open(filename, access = 'rb') { |fp| ... }
    The (({open})) method opens an existing shapefile. 

    Argument ((|filename|)) should be
    either fullname (like "(({basename.shp}))")
    or just basename (like "(({basename}))").
    Suffix (({.shp})) must be lowercase if specified.
    Both (({.shp})) and (({.shx})) files must exist at the specified path.

    Argument ((|access|)) are passed to (({fopen(3)})) through
    ((<ShapeLib>)) (({SHPOpen()})) function.
    Either '(({rb}))' or '(({rb+}))' should be used.

    If a block is given to these constructors,
    the opened shapefile object is yielded like File::open.
    The shapefile is closed at the end of block,
    and the constructor returns (({nil})).

	# example
	ShapeLib::ShapeFile.open(filename) { |fp|
	   print "#{filename} has #{fp.size} records.\n"
	}

--- ShapeFile::new filename, shapetype, attrs
--- ShapeFile::new(filename, shapetype, attrs) { |fp| ... }
    The (({new})) method creates a shapefile.
    All three files ((({.shp})), (({.shx})), and (({.dbf}))) are created.
    Argument ((|shapetype|)) specifies the ((<type of shapes>))
    to be stored in the shapefile.
    Argument ((|attrs|)) is an array of arrays
    that specifies ((|field_name|)), ((|field_type|)), ((|width|)),
    and optional ((|decimal|)) as in
    ((<shapefile.add_field|shapefile.add_field field_name, field_type, width, decimal = 0>))
    .

       # example
       fp = ShapeFile::new('city.shp', :Point,
          [['name', :String, 32],
           ['population', :Integer, 9],
           ['height', :Float, 9, 1]])

=== instance methods

--- shapefile.add_field field_name, field_type, width, decimal = 0
    Adds an attribute field to the shapefile (that must be newly created by
    ((<ShapeFile::new|ShapeFile::new filename, shapetype, attrs>))).
    Argument ((|field_name|)) is a String for field name;
    ((|field_type|)) specifies ((<field type>));
    ((|width|)) is nonnegative Integer for field width in characters; and
    ((|decimal|)) is nonnegative Integer for decimal points.
    Only Float field requires ((|decimal|)); otherwise it can be missing.

      # example
      fp.add_field 'name', :String, 32
      fp.add_field 'height', "Float", 9, 1

--- shapefile.each { |shape| ... }
    yields all shapes in the shapefile.
    Equivalent to the following code:

      shapefile.rewind
      while shape = shapefile.read
        ...
      end

--- shapefile.field_count
    returns the number of attribute fields made on the DBF file.

--- shapefile.field_decimals field
    returns the width of the ((<attribute field>)).
    Zero is returned for non-Float fields.

--- shapefile.field_index name
    returns the index of the attribute field matching ((|name|)), or nil on failure.

--- shapefile.field_name index
    returns the name of the attribute field with ((|index|)), or nil on failure.

--- shapefile.field_type field
    returns the type of the ((<attribute field>)).
    The result is Symbol
    (one of :String, :Integer, :Float, :Logical, or :Invalid)
    or nil on failure.

--- shapefile.field_width field
    returns the width of the ((<attribute field>)).

--- shapefile.fields
    returns an Array containing a list of attribute field name.

--- shapefile.maxbound
    returns the maximum values for x, y, m, z coordinates.
    Note: the order is M then Z, not Z then M.

--- shapefile.minbound
    returns the minimum values for x, y, m, z coordinates.

--- shapefile.read rec = -1
    reads the shape/record ((|rec|)) from shapefile.
    If ((|rec|)) is missing,
    the next record to previously read one
    (or the first record #0 for the first time) is used.
    An instance of ((<class Shape>)) is returned,
    including all attributes written on the DBF.
    On error (({nil})) is returned.

--- shapefile.rewind
    After that, ((<shapefile.read|shapefile.read rec = -1>))
    without argument will return the first shape/record of the shapefile.

--- shapefile.shape_type
    returns the type of shape by Symbol.

--- shapefile.size
    returns the number of records (or shapes) in the shapefile.
    ((-If DBF file has different number of records,
    warning message is shown in $VERBOSE mode.-))

--- shapefile.write shape, rec = nil
    writes ((|shape|)) into shapefile at record number ((|rec|)).
    The ((|shape|)) must be a instance of ((<class Shape>)) or subclass.
    Attributes attached to ((|shape|)) are stored into DBF file.
    If ((|rec|)) is missing, the record is appended at the end of shapefile.

--- shapefile.close
    The (({close})) method flushes all information
    into the shapefile and release related resources.
    After that the instance cannot be used.

    It is recommended to call it at the end of processing
    to reduce the risk of trouble,
    although it is called automatically
    at the time of garbage collecting or at the end of interpreter.

== class Shape
Shape consists of a geometric shape (written in SHP file)
and attributes attached to the shape (written in DBF file).
Instance of these subclasses are obtained from
((<ShapeFile#read|shapefile.read rec = -1>))
or constructor like ((<Point::new|class Point>)).

Every instance of the class Shape belongs to one of subclasses,
depending on the type of shape. 

=== structure of subclasses

* ((<class Shape>))
  * ((<class Point>))
    * ((<class PointM>))
      * ((<class PointZ>))
  * ((<class Arc>))
      * ((<class ArcM>))
	  * ((<class ArcZ>))
  * ((<class Polygon>)) 
      * ((<class PolygonM>)) 
	  * ((<class PolygonZ>)) 
  * ((<class MultiPoint>)) 
      * ((<class MultiPointM>)) 
	  * ((<class MultiPointZ>)) 
  * ((<class MultiPatch>)) 

=== class method
--- Shape::new hash
    Argument ((|hash|)) is a hash (or ((<something behaves like a hash>)))
    that contains the attributes of the shape.
    Hash returned by ((<shape.to_h>)) can be used as ((|hash|)).

=== instance methods
--- shape [field]
    value of the ((<attribute|attribute field>)) ((|field|))
    related to the shape.
--- shape [field] = value
    sets ((<attribute|attribute field>)) ((|field|)) to ((|value|)).
--- shape.inspect
    currently similar to ((<shape.to_h>))(({.to_s})).
--- shape.maxbound
    returns the maximum values for x, y, m, z coordinates.
    Note: the order is M then Z, not Z then M.
--- shape.minbound
    returns the minimum values for x, y, m, z coordinates.
--- shape.part_start
    returns an array of Integer that indicates beginning offset
    for each part in ((<shape.xvals>)) etc.
--- shape.part_type
    returns an array containing ((<part type>)) by Symbol.
    Point or multipoint returns (({nil})).
--- shape.rewind_polygon
    "This function will reverse any rings necessary
    in order to enforce the shapefile restrictions
    on the required order of inner and outer rings
    in the Shapefile specification.
    It returns (({true})) if a change is made and
    (({false})) if no change is made.
    Only polygon objects will be affected though any object may be passed."
    ((-taken from ((<ShapeLib>)) document.-))
--- shape.shape_id
    returns the number of record by which the shape is read from a shapefile.
    Nil is returned if the shape is not read from file.

--- shape.shape_type
    returns the ((<type of shape|type of shapes>)) by Symbol.

--- shape.to_h
    returns a hash that contains all attributes and geometry information.
    Each attribute have a key of String,
    while geometry information has keys of Symbol.
	pt = ShapeLib::Point::new(1, 2, 'name' => 'Madison')
	pt.to_h => {"name"=>"Madison", :zvals=>[0.0], :n_parts=>0,
	    :n_vertices=>1, :maxbound=>[1.0, 2.0, 0.0, 0.0],
	    :part_start=>nil, :part_type=>nil, :xvals=>[1.0],
	    :minbound=>[1.0, 2.0, 0.0, 0.0], :shape_type=>:Point,
	    :yvals=>[2.0], :mvals=>[0.0], :shape_id=>nil}

--- shape.to_s
    current implementation is identical to ((<shape.wkt>)).
    Note that this lose information.

--- shape.wkt
    returns WKT (well known text) representation of geometry
    as defined in ((<ISO 19125-1>)).
    ((*Caution: current implementation does not always handle Polygon[MZ] and MultiPatch correctly.*))

      ShapeLib::Arc.new([1, 2], [3, 4], [5, 6]).wkt
      => "LINESTRING(1 2, 3 4, 5 6)"
      ShapeLib::Arc.new([[1, 2], [3, 4]], [[5, 6], [7, 8]]).wkt
      => "MULTILINESTRING((1 2, 3 4), (5 6, 7 8))"

--- shape.xvals
    returns an array of x values.

      ShapeLib::Arc.new([1, 2], [3, 4], [5, 6]).xvals
      => [1.0, 3.0, 5.0]

    Multipart shape results flattened array (all parts joined).

      ShapeLib::Arc.new([[1, 2], [3, 4]], [[5, 6], [7, 8]]).xvals
      => [1.0, 3.0, 5.0, 7.0]

--- shape.yvals
    returns an array of y values.

      ShapeLib::Arc.new([1, 2], [3, 4], [5, 6]).xvals
      => [2.0, 4.0, 6.0]

--- shape.mvals
    returns an array of m values. Nil is returned for missing value.

      ShapeLib::Arc.new([1, 2], [3, 4], [5, 6]).mvals
      => [nil, nil, nil]
      ShapeLib::ArcM.new([1, 2, 3], [4, 5, 6], [7, 8, 9]).mvals
      => [3, 6, 9]

--- shape.zvals
    returns an array of z values.

== class Point

Point is a geometric point that has two coordinates x and y.

--- Point::new x, y, attrs = {}
    creates a new point instance with coordinates ((|x|)) and ((|y|))
    (both are converted to Float).
    Optional argument ((|attrs|)) is a hash containing attributes
    such as (({{"key1"=>value1, "key2"=>value2, ...}})).
--- point.x
    returns a Float x value.
--- point.y
    returns a Float y value.
--- point.m
    returns a Float m (height) value.
--- point.z
    returns a Float z (elevation) value.

== class PointM
Point is a geometric point that has three coordinates x, y and m.
--- PointM::new x, y, m, attrs = {}

== class PointZ
Point is a geometric point that has four coordinates x, y, m and z.
--- PointZ::new x, y, m, z, attrs = {}

== class Arc
Arc is consists of a set of (two or more) points.
Each point has two coordinates x and y.
It is also called polyline.

--- Arc::new point, point, ...
    Constructs a single arc.
    ((|point|)) is array of numerals, or ((<class Point>)).

--- Arc::new [point, point, ...], [point, point, ...], ...
    Constructs a multipart arc that represents two disconnected polyline.

== class ArcM
ArcM is similar to Arc, but points also have m (measure) coordinates.

== class ArcZ
ArcZ is similar to ArcM, but points also have z coordinates.

== class Polygon
Polygon is one (or more) closed arc.
--- Polygon::new point, point, ...
    Constructs a object with single polygon.
--- Polygon::new [point, point, ...], [point, point, ...], ...
    Constructs a object with multiple polygons.

== class PolygonM
PolygonM is similar to Polygon, but points also have m (measure) coordinates.

== class PolygonZ
PolygonZ is similar to PolygonM, but points also have z coordinates.

== class MultiPoint
MultiPoint is a set of points.
--- MultiPoint::new point, point, ...
--- MultiPoint::new [point, point, ...]
    Above two are the same.

== class MultiPointM
MultiPointM is similar to MultiPoint, but points also have m (measure) coordinates.

== class MultiPointZ
MultiPointZ is similar to MultiPointM, but points also have z coordinates.

== class MultiPatch
--- MultiPatch::new [part_type, point, point, ...], [part_type, point, ...], ...
    Constructs a MultiPatch object.
    Note that the first element of each array must be ((<part type>)).

== constants
=== Field Type Constants
--- ShapeLib::String
--- ShapeLib::Integer
--- ShapeLib::Float

== Appendix A: C to Ruby equivalence table
* DBFHandle:
  ((<class ShapeFile>)) contains DBFHandle.
* SHPHandle:
  ((<class ShapeFile>)) contains SHPHandle.
* SHPObject:
  ((<class Shape>)) contains SHPObject and attributes stored in DBF.
  Access to members of the structure can be translated using ((<shape.to_h>)).
* SHPOpen():
  called in ((<ShapeFile::open|ShapeFile::open filename, access = 'rb'>))
* SHPGetInfo():
  ((<shapefile.size>)),
  ((<shapefile.shape_type>)),
  ((<shapefile.minbound>)), and
  ((<shapefile.maxbound>)).
* SHPReadObject():
  ((<shapefile.read|shapefile.read rec = -1>))
* SHPClose():
  ((<shapefile.close>))
* SHPCreate():
  called in ((<ShapeFile::new|ShapeFile::new filename, shapetype, attrs>))
* SHPCreateSimpleObject() and SHPCreateObject():
  ((<Shape::new|Shape::new hash>)) and (({new})) method of its subclasses
* SHPComputeExtents():
  there is no need to call it, since the current version of
  ruby-shapelib does not allow users to change geometry of a shape
  once it is created.
* SHPWriteObject():
  ((<shapefile.write|shapefile.write shape, rec = nil>))
* SHPDestroyObject():
  called automatically while the ruby interpreter does garbage collection.
* SHPRewindObject():
  ((<shape.rewind_polygon>))
* DBFOpen():
  called in ((<ShapeFile::open|ShapeFile::open filename, access = 'rb'>))
* DBFCreate():
  called in ((<ShapeFile::new|ShapeFile::new filename, shapetype, attrs>))
* DBFGetFieldCount():
  ((<shapefile.field_count>))
* DBFGetRecordCount():
  There is no need to call it if shapefile is properly created,
  i.e. *.SHP and *.DBF files have the same number of records.
  See also ((<shapefile.size>)).
* DBFGetFieldIndex():
  ((<shapefile.field_index|shapefile.field_index name>))
* DBFGetFieldInfo():
  ((<shapefile.field_decimals|shapefile.field_decimals field>)),
  ((<shapefile.field_name|shapefile.field_name index>)),
  ((<shapefile.field_type|shapefile.field_type field>)), and
  ((<shapefile.field_width|shapefile.field_width field>))
* DBFAddField():
  ((<shapefile.add_field|shapefile.add_field field_name, field_type, width, decimal = 0>))
* DBFReadIntegerAttribute(), DBFReadDoubleAttribute(), and
  DBFReadStringAttribute():
  ((<shape [field]>))
* DBFIsAttributeNULL():
  called in ((<shape [field]>)).
  Nil is returned if the attribute is null.
* DBFWriteIntegerAttribute(), DBFWriteDoubleAttribute(), and
  DBFWriteStringAttribute():
  ((<shape [field] = value>))
* DBFWriteNULLAttribute():
  called if (({nil})) is given to ((<shape [field] = value>)).
* DBFGetNativeFieldType():
  not implemented in ruby-shapelib.
  Does anyone know how to interpret the return value properly?

== Appendix B: Flexible argument
Some method arguments take various objects for convenience.
This appendix discusses the detail of acceptable value.

: should be Float
  Any object that respond to (({to_f})) is converted to Float.
  String is not acceptable.
  (({nil})) is treated as -2e-38 (NODATA) for ((|m|)) (measure) coordinate,
  or as 0.0 otherwise.

: type of shapes
  For ((<ShapeFile::new|ShapeFile::new filename, shapetype, attrs>))
  and ((<Shape::new|Shape::new hash>)),
  acceptable values for ((|shapetype|)) are:
  * Class object like ShapeLib::Point (listed in ((<structure of subclasses>)))
  * String such as (({"Point"})) or (({"PolygonZ"}))
    (the same list, but do not include "ShapeLib::")
  * Symbol such as (({:Point})) or (({:PolygonZ}))
    (result of ((<shapefile.shape_type>)) and ((<shape.shape_type>))),
    or
  * Integer code
    (please see ((<ESRI Whitepaper>)))

: field type
  For ((<shapefile.add_field|shapefile.add_field field_name, field_type, width, decimal = 0>)),
  acceptable values for ((|field_type|)) are:
  * String "String", "Integer", or "Float", or
  * Symbol :String, :Integer, or :Float
    (result of ((<shapefile.field_type|shapefile.field_type field>))).
  * Integer constants listed ((<above|Field Type Constants>)).

: part type
  acceptable values are
  * String "TriangleStrip", "TriangleFan", "OuterRing", "InnerRing",
    "FirstRing", or "Ring"
  * corresponding Symbol :TriangleStrip etc.

: attribute field
  For many shapefile methods like ((<shapefile.field_type field>)),
  both String name and Integer index are acceptable.

: something behaves like a hash
  For ((<Shape::new hash>)), the argument ((|hash|))
  * must respond to [((|key|))] method with a Symbol as a ((|key|))
  * should respond to (({to_a})) method, so that attributes be loaded 


== References
: ShapeLib
  ((<URL:http://shapelib.maptools.org/shp_api.html>))
  ((<URL:http://shapelib.maptools.org/dbf_api.html>))
: ESRI Whitepaper
  ((<URL:http://www.esri.com/library/whitepapers/pdfs/shapefile.pdf>))
  or ((<URL:http://shapelib.maptools.org/dl/shapefile.pdf>))
: ISO 19125-1
  ((<Found on opengeospatial.org|URL:http://portal.opengeospatial.org/modules/admin/license_agreement.php?suppressHeaders=0&access_license_id=3&target=http://portal.opengeospatial.org/files/index.php?artifact_id=13227>)).
=end

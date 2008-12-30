require 'shapelib'
require 'libtest.rb'

def test1
    testblock('test1') do 
	file_delete(*%w(test1.dbf test1.shp test1.shx))
	s = ShapeLib::ShapeFile.new('test1.shp', :Point, [["id", :Integer, 8]])
	assert(s.field_count, 1)
	s.add_field 'name', ShapeLib::String, 12
	assert(s.field_count, 2)
	assert(s.field_index('name'), 1)
	assert(s.field_type('name'), :String)
	assert(s.field_type(1), :String)
	s.add_field 'k', ShapeLib::Integer, 3
	assert(s.field_count, 3)
	s.add_field 'y', "Float", 8, 3
	assert(s.field_index('y'), 3)
	assert(s.field_type('y'), :Float)
	assert(s.field_count, 4)
	assert(s.size, 0)

	shape = ShapeLib::Point.new(100, 101, 'id' => 234)
	shape['name'] = 'foo'
	shape['k'] = 18
	fval = shape['y'] = 31.415926
	assert(s.write(shape), 0)
	assert(s.size, 1)
	assert(shape.x, 100.0)

	assert(s.fields, %w(id name k y))
	assert(s.field_type(1), :String)
	assert(s.field_name(1), 'name')

	assert(shape['id'], 234)
	assert(shape['name'], 'foo')
	assert(shape['k'], 18)
	assert(shape['y'], fval)
	assert(shape['unspecified'], nil)

	shape = ShapeLib.new_point(200, 201)
	assert(s.write(shape), 1)
	assert(s.size, 2)

	assert(shape['id'], nil)
	assert(shape.x, 200.0)

	shape = s.read(0)
	assert(shape.shape_id, 0)
	assert(shape['id'], 234)
	assert(shape['name'], 'foo')
	assert(shape['k'], 18)
	# note that only 3 digits after decimal point is conserved.
	assert(shape['y'], 31.416)
	assert(shape['unspecified'], nil)
	assert(shape['unspecified2'], nil)

	assert s.maxbound, [200, 201, 0, 0]

	s.close

	assert_file 'test1.dbf', <<EOF
035f071a02000000a10020000000000000000000000000000000000000000000
69640000000000000000004e0000000008000000000000000000000000000000
6e616d650000000000000043000000000c000000000000000000000000000000
6b000000000000000000004e0000000003000000000000000000000000000000
79000000000000000000004e0000000008030000000000000000000000000000
0d202020202020323334666f6f202020202020202020203138202033312e3431
36202a2a2a2a2a2a2a2a0000000000000000000000002a2a2a2a2a2a2a2a2a2a
2a
EOF
	assert_file 'test1.shp', <<EOF
0000270a00000000000000000000000000000000000000000000004ee8030000
0100000000000000000059400000000000405940000000000000694000000000
0020694000000000000000000000000000000000000000000000000000000000
00000000000000010000000a0100000000000000000059400000000000405940
000000020000000a0100000000000000000069400000000000206940
EOF
	assert_file 'test1.shx', <<EOF
0000270a00000000000000000000000000000000000000000000003ae8030000
0100000000000000000059400000000000405940000000000000694000000000
0020694000000000000000000000000000000000000000000000000000000000
00000000000000320000000a000000400000000a
EOF

	file_delete(*%w(test1.dbf test1.shp test1.shx))
    end
end

def test2
    testblock('test2') do 
	file_delete(*%w(test2.dbf test2.shp test2.shx))
	s = ShapeLib::ShapeFile.new('test2.shp', ShapeLib::PointM)
	assert(s.class.to_s, "ShapeLib::ShapeFile")
	assert(s.field_count, 0)
	s.add_field 'name', ShapeLib::String, 12
	assert(s.field_count, 1)
	assert(s.size, 0)

	shape = ShapeLib.new_point(301, 302, 303)
	assert(shape.class.to_s, "ShapeLib::PointM")
	s.write(shape)
	assert(s.size, 1)
	assert(shape.m, 303)

	shape = ShapeLib::PointM.new(401, 402, 403)
	assert(shape.class.to_s, "ShapeLib::PointM")
	s.write(shape)
	assert(s.size, 2)

	s.close
	assert_file 'test2.shp', <<EOF
0000270a000000000000000000000000000000000000000000000056e8030000
150000000000000000d072400000000000e07240000000000010794000000000
00207940000000000000000000000000000000000000000000f0724000000000
00307940000000010000000e150000000000000000d072400000000000e07240
0000000000f07240000000020000000e15000000000000000010794000000000
002079400000000000307940
EOF

        sx = ShapeLib::ShapeFile.open('test2.shp')
	assert sx.shape_type, :PointM
	assert sx.minbound, [301.0, 302.0, 303.0, 0]
	assert sx.maxbound, [401.0, 402.0, 403.0, 0]
	sx.close

	file_delete(*%w(test2.dbf test2.shp test2.shx))
    end
end

def testA1
    testblock('testA1') do 
	file_delete(*%w(testa1.dbf testa1.shp testa1.shx))
	s = ShapeLib::ShapeFile.new('testa1.shp', "Arc")
	assert(s.class.to_s, "ShapeLib::ShapeFile")
	assert(s.field_count, 0)
	s.add_field 'name', ShapeLib::String, 12
	assert(s.field_count, 1)
	assert(s.size, 0)
	
	p1 = ShapeLib.new_point(1.1, 1.2)
	p2 = ShapeLib.new_point(2.1, 2.2)
	p3 = ShapeLib.new_point(3.1, 3.2)
	shape = ShapeLib::Arc.new([p1, p2, p3])
	assert(shape.class.to_s, 'ShapeLib::Arc')
	assert(shape.n_parts, 1)
	assert(shape.n_vertices, 3)
	assert(shape.shape_id, nil)
	assert(shape.shape_type, :Arc)
	assert(s.write(shape), 0)

	arc2 = [[101, 102], [111, 112], [121, 122]]
	shape = ShapeLib::Arc.new([p1, p3], arc2)
	assert(shape.n_parts, 2)
	assert(s.write(shape), 1)

	assert(shape.shape_id, nil)
	assert(s.read(1).shape_id, 1)

	file_delete(*%w(testa1.dbf testa1.shp testa1.shx))
    end
end

def test3
    testblock('test3') do
	pt = ShapeLib::Point.new 4324434.13545, 3495423412.4324324
	assert(pt.wkt, "POINT(4324434.135 3495423412)")
	assert(pt.part_type, nil)
	mp = ShapeLib::MultiPoint.new [1, 2], [3, 4], [5, 6]
	assert(mp.wkt, "MULTIPOINT(1 2, 3 4, 5 6)")

	ar = ShapeLib::Arc.new [1, 2], [3, 4], [5, 6]
	assert(ar.wkt, "LINESTRING(1 2, 3 4, 5 6)")
	assert(ar.xvals, [1, 3, 5])
	assert(ar.yvals, [2, 4, 6])
	ar = ShapeLib::Arc.new([[1, 2], [3, 4], [5, 6]])
	assert(ar.wkt, "LINESTRING(1 2, 3 4, 5 6)")
	ar = ShapeLib::Arc.new([[1, 2], [3, 4], [5, 6]], [[7, 8], [9, 10]])
	assert(ar.wkt, "MULTILINESTRING((1 2, 3 4, 5 6), (7 8, 9 10))")
	assert ar.xvals, [1, 3, 5, 7, 9]

	ar2 = ShapeLib::Shape.new(arh = ar.to_h)
	assert(ar2.class.to_s, "ShapeLib::Arc")
	assert(ar2.to_h, arh)
	assert(ar2.wkt, ar.wkt)

	arm = ShapeLib::ArcM.new [1, 2, 3], [4, 5, 6], [7, 8, 9]
	assert arm.mvals, [3, 6, 9]
	arm = ShapeLib::ArcM.new [1, 2], [4, 5], [7, 8]
	assert arm.mvals, [nil, nil, nil]

	pl = ShapeLib::Polygon.new [1, 2], [3, 4], [5, 6.5], [1, 2]
	assert(pl.wkt, "POLYGON((1 2, 3 4, 5 6.5, 1 2))")
	assert(pl.rewind_polygon, true)
	assert(pl.wkt, "POLYGON((1 2, 5 6.5, 3 4, 1 2))")
	pl = ShapeLib::Polygon.new [[1, 2], [3, 4], [5, 6.5], [1, 2]]
	assert(pl.wkt, "POLYGON((1 2, 3 4, 5 6.5, 1 2))")
	pl = ShapeLib::Polygon.new([[1, 2], [3, 4], [5, 6.5], [1, 2]],
	    [[7, 8], [9, 10], [11, 12.5], [7, 8]])
	assert(pl.part_type, [:Ring, :Ring])
	assert(pl.wkt,
	    "POLYGON((1 2, 3 4, 5 6.5, 1 2), (7 8, 9 10, 11 12.5, 7 8))")

	pl['abbrev'] = 'WI'
	pl['state'] = 'Wisconsin'
	pl['update'] = '2006-02-16'
	assert ShapeLib::Shape.new(pl.to_h).to_h, pl.to_h

	mq = ShapeLib::MultiPatch.new :OuterRing, [1, 2], [3, 4], [5, 6.5], [1, 2]
	assert(mq.wkt, "POLYGON((1 2, 3 4, 5 6.5, 1 2))")
    end
end

def test4
    testblock('test4') do 
	file_delete(*%w(test4.dbf test4.shp test4.shx))
	s = ShapeLib::ShapeFile.new('test4.shp', :PointM)
	assert(s.class.to_s, "ShapeLib::ShapeFile")
	assert(s.field_count, 0)
	assert(s.size, 0)
	# shape = ShapeLib.new_point(301, 302, 303, 'foo' => 123)
	shape = ShapeLib.new_point(301, 302, 303)
	assert(s.write(shape), 0)
	shape = ShapeLib::PointM.new(401, 402, 403)
	assert(s.write(shape), 1)
	s.close

	fp = ShapeLib::ShapeFile.open('test4')
	assert fp.size, 2
	shape = fp.read(0)
	assert(shape.class.to_s, "ShapeLib::Shape")
	assert fp.size, 2
	fp.close

	ShapeLib::ShapeFile.open('test4.shp') do |fp|
	    shape = fp.read
	    assert(shape.shape_id, 0)
	    shape = fp.read
	    assert(shape.shape_id, 1)
	    assert(fp.read, nil)
	    i = 0
	    fp.each do |shape|
		assert shape.shape_id, i
		i += 1
	    end
	    assert i, 2
	end

	file_delete(*%w(test4.dbf test4.shp test4.shx))
    end
end


exit(test1 + test2 + testA1 + test3 + test4)

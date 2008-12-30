require 'shapelib'

class Sample1
    include ShapeLib
    
    def initialize
    end

    def layer1
	fp = ShapeFile.new('ts1stn.shp', :Point)
	fp.add_field 'name', :String, 24
	fp.add_field 'wdir', :String, 6 # 4
	fp.add_field 'wspd', :Integer, 6 # 2
	fp.add_field 'weather', :String, 16
	fp.add_field 'pres', :Integer, 6 # 4
	fp.add_field 'temp', :Integer, 6 # 3
	fp.write new_point(124.17, 24.33,
	    'name' => 'Ishigakijima', 'wdir' => 'S', 'wspd' => 3,
	    'weather' => 'shower', 'pres' => 1014, 'temp' => 24)
	csv = <<EOF
127:41.1, 26:12.4, Naha, S, 4, partly cloudy, 1015, 21
131.23, 25.83, Minamidaito Jima, SSW, 2, partly cloudy, 1017, 20
129:29.7, 28:22.7, Naze, SW, 2, partly cloudy, 1017, 20
130.72, 31.80, Kagoshima, WSW, 1, shower, 1013, 18
128:49.6, 32:41.6, Fukue, NNE, 3, cloudy, 1014, 11
129:17.5, 34:11.8, Izuhara, NNW, 2, cloudy, 1016, 10
133:00.6, 32:43.3, Ashizuri Misaki, W, 2, cloudy, 1011, 17
134:10.6, 33:15.1, Muroto Misaki, NNE, 3, fog, 1012, 13
132:46.6, 33:50.6, Matsuyama, SE, 1, rain, 1014, 10
132:04.2, 34:53.8, Hamada, ENE, 1, cloudy, 1015, 8
133.3, 36.2, Saigo, NNE, 2, shower, 1016, 7
135.6, 34.7, Osaka, NNE, 2, cloudy, 1014, 9
135:45.6, 33:27.0, Shio no Misaki, NE, 1, shower, 1012, 13
139.8, 33.1, Hachijo Jima, SW, 4, shower, 1013, 18
139.4, 34.8, Izu Oshima, NNE, 4, shower, 1014, 9
138:12.7, 34:36.2, Omaezaki, NE, 2, rain, 1012, 12
140:51.4, 35:44.3, Choshi, NNE, 5, rain, 1014, 7
139:03.6, 36:24.3, Maebashi, WNW, 1, rain, 1015, 8
140:54.2, 36:56.8, Onahama, N, 3, rain, 1017, 5
136:53.7, 37:23.5, Wajima, ENE, 1, rain, 1016, 5
138:14.3, 38:01.7, Aikawa, SE, 2, rain, 1016, 4
140.9, 38.2, Sendai, NNW, 1, snow-rain, 1018, 1
141:57.9, 39:38.8, Miyako, N, 2, snow, 1020, 1
140.2, 39.6, Akita, NNE, 1, cloudy, 1018, 0
140.8, 41.8, Hakodate, NNE, 3, cloudy, 1019, -4
142:46.6, 42:09.7, Urakawa, NNE, 2, cloudy, 1019, -5
145:35.1, 43:19.8, Nemuro, CALM, 0, cloudy, 1019, -5
141:40.7, 45:24.9, Wakkanai, W, 3, cloudy, 1018, -4
151, 46, Poronaisk, W, 3, cloudy, 1018, -4
156, 50, Severo-Kurilsk, WSW, 3, partly cloudy, 1014, -16
135.17, 48.52, Habarovsk, CALM, 0, partly cloudy, 1014, -19
135:48, 44:21, Rudnaya Pristan, W, 1, cloudy, 1019, -13
131.93, 43.12, Vladivostok, SE, 3, cloudy, 1018, -5
126.97, 37.57, Seoul, NNE, 1, partly cloudy, 1019, 1
131, 37, Ulleung-do, NE, 1, cloudy, 1018, 3
128.93, 35.18, Pusan, NNE, 3, cloudy, 1016, 6
127, 35, Mokpo, NNE, 3, cloudy, 1019, 3
126.50, 33.50, Cheju-do, NNE, 1, cloudy, 1017, 9
121.55, 25.07, Taipei, ESE, 1, cloudy, 1013, 21
120.35, 22.58, Kaohsiung, N, 1, cloudy, 1013, 24
125.22, 43.90, Changchun, WNW, 3, clear, 1024, -13
116.28, 39.93, Beijing, NNW, 4, partly cloudy, 1031, -2
121.63, 38.90, Dalian, NNW, 3, partly cloudy, 1024, -1
120.33, 36.07, Qingdao, NNW, 4, fog, 1024, 2
121.4, 31.2, Shanghai, N, 3, rain, 1021, 7
114.13, 30.62, Wuhan, NNW, 2, rain, 1025, 8
118.08, 24.48, Xiamen, CALM, 0, fog, 1014, 18
114.02, 22.20, Hong Kong, ESE, 2, partly cloudy, 1014, 21
120.53, 18.18, Laoag, CALM, 0, clear, 1012, 22
121.00, 14.52, Manila, WNW, 1, partly cloudy, 1013, 25
142.18, 27.08, Chichi Jima, NW, 1, clear, 1020, 16
133.97, 24.30, Minami Tori Shima, E, 4, X, 1021, 23
138:43.6, 35:21.6, Fujisan, X, X, X, X, -4
126, 29, SHIP,  NW, 5, rain, 1015, X
148, 21, SHIP,   E, 3, cloudy, 1018, X
139, 31, SHIP,  SW, 7, partly cloudy, 1016, X
136, 39, SHIP, ENE, 5, partly cloudy, 1017, X
146, 36, SHIP, ENE, 5, rain, 1013, X
EOF
	for line in csv.split(/\n/)
	    x, y, name, wdir, wspd, weather, pres, temp = line.split(/, */)
	    x = $1.to_i + $2.to_f / 60 if /(\d+):(\d+(\.\d)?)/ === x
	    y = $1.to_i + $2.to_f / 60 if /(\d+):(\d+(\.\d)?)/ === y
	    x = Float(x)
	    y = Float(y)
	    wdir = nil if wdir == 'X'
	    weather = nil if weather == 'X'
	    wspd = (wspd == 'X' ? nil : Integer(wspd))
	    pres = (pres == 'X' ? nil : Integer(pres))
	    temp = (temp == 'X' ? nil : Integer(temp))
	    fp.write new_point(x, y,
		'name' => name, 'wdir' => wdir, 'wspd' => wspd,
		'weather' => weather , 'pres' => pres, 'temp' => temp)
	end
	fp.close
    end

    def layer2
	fp = ShapeFile.new('ts1sys.shp', :Point)
	fp.add_field 'class', 'String', 16
	fp.add_field 'symbol', 'String', 3
	fp.add_field 'pres', 'Integer', 4
	fp.add_field 'dir', 'String', 4
	fp.add_field 'speed', 'Integer', 3
	fp.write new_point(169, 45, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 992, 'dir' => 'NNE', 'speed' => 20)
	fp.write new_point(177, 58, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 996, 'dir' => 'N', 'speed' => 10)
	fp.write new_point(130, 41, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 1016, 'dir' => 'ENE', 'speed' => 10)
	fp.write new_point(134, 33, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 1012, 'dir' => 'ENE', 'speed' => 20)
	fp.write new_point(152, 57, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 1000, 'dir' => 'WSW', 'speed' => 5)
	fp.write new_point(156, 48, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 1004, 'dir' => 'E', 'speed' => 10)
	fp.write new_point(139,  3, 'class' => 'Low', 'symbol' => 'L',
	    'pres' => 1008, 'dir' => 'STNR', 'speed' => 0)
	fp.write new_point(138, 43, 'class' => 'High', 'symbol' => 'H',
	    'pres' => 1020, 'dir' => 'E', 'speed' => 15)
	fp.write new_point(164, 29, 'class' => 'High', 'symbol' => 'H',
	    'pres' => 1024, 'dir' => 'E', 'speed' => 20)
	fp.close
    end

    def layer3
	fp = ShapeFile.new('ts1frnt.shp', :Arc)
	fp.add_field 'class', 'String', 16

	arc = Arc.new [[169, 45], [172, 44], [175, 41]]
	arc['class'] = 'occlusion'
	fp.write arc

	arc = Arc.new [[175, 41], [177, 39], [178, 37]]
	arc['class'] = 'warm'
	fp.write arc

	arc = Arc.new [[175, 41], [171, 38], [164, 36]]
	arc['class'] = 'cold'
	fp.write arc

	arc = Arc.new [[119, 27], [128, 29], [134, 33], [139, 34],
	    [142, 35], [150, 35]]
	arc['class'] = 'stationary'
	fp.write arc

	fp.close
    end

    def layer4
	fp = ShapeFile.new('ts1isop.shp', :Arc)
	fp.add_field 'pres', 'Integer', 4
	arc = Arc.new [[168, 19], [150, 18], [141, 19], [129, 22], [133, 29], [147, 34], [149, 36], [141, 37], [128, 34], [122, 28], [110, 25]]
	arc['pres'] = 1016
	fp.write arc
	fp.close
    end

    def run
	layer1
	layer2
	layer3
	layer4
    end

end

Sample1.new.run

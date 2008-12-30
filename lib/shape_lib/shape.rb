module ShapeLib
  class Shape
    def each
      latlngs.each do |l|
        yield l
      end
    end
    
    def latlngs
      @latlngs ||= yvals.zip(xvals)
    end
  end
end
      
require_relative 'svg_file'

module DHE
  class Control
    attr_reader :faceplate, :width, :height, :top, :right, :bottom, :left, :x, :y

    def initialize(faceplate:, top:, right:, bottom:, left:, x: (right + left) / 2.0, y: (bottom + top) / 2.0)
      @faceplate = faceplate
      @top = top
      @right = right
      @bottom = bottom
      @left = left
      @width = right - left
      @height = bottom - top
      @x = x
      @y = y
    end

    def self.centered(x:, y:, width:, height: width)
      {left: x - width / 2, right: x + width / 2, top: y - height / 2, bottom: y + height / 2, }
    end

    def draw_control(svg:, **options)
      draw(svg: svg, x: width / 2.0, y: height / 2.0, **options)
    end

    def draw_faceplate(svg:)
      draw(svg: svg, x: @x, y: @y)
    end

    def svg_file(path:, has_text: false)
      SvgFile.new(path: path,
                  width: "#{width}mm", height: "#{height}mm", viewBox: "0 0 #{width} #{height}",
                  has_text: has_text) do |svg|
        yield svg
      end
    end
  end

  class RoundControl < Control
    attr_reader :diameter

    def initialize(faceplate:, x:, y:, diameter:)
      super(faceplate: faceplate, **Control::centered(x: x, y: y, width: diameter, height: diameter))
      @diameter = diameter
    end

    def radius
      diameter / 2
    end
  end
end

require_relative '../control'
require_relative '../dimensions'

module DHE
  class Counter < Control
    def initialize(faceplate:, x:, y:, name:, labels:, enabled:, selection:)
      @name = name
      @slug = "counter-#{@name}"
      @button = Button.new(faceplate: faceplate, x: x, y: y)
      @label_offset = @button.radius + PADDING
      @labels = labels.map { |label| Label.new(faceplate: faceplate, x: x, y: y - @label_offset, text: label, size:
          :small) }
      bottom = y + (@button.y - @labels[0].top)
      super(faceplate: faceplate, x: x, y: y, top: @labels[0].top, right: @button.right, bottom: bottom, left:
          @button.left)
      @enabled = enabled
      @selection = selection
    end

    def draw(svg:, x:, y:, selection: @selection)
      @labels[selection - 1].draw(svg: svg, x: x, y: y - @label_offset)
      @button.draw(svg: svg, x: x, y: y)
    end

    def draw_faceplate(svg:)
      return unless @enabled
      draw(svg: svg, x: @x, y: @y)
    end

    def svg_files
      (0...@labels.size).map do |index|
        selection = index + 1
        path = faceplate.slug / "#{@slug}-#{selection}"
        svg_file(path: path, has_text: true) do |svg|
          draw_control(svg: svg, selection: selection)
        end
      end
    end
  end
end

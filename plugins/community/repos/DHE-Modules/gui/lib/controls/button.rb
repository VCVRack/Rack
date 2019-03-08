require_relative '../control'

module DHE
  class Button < RoundControl
    DIAMETER = 6.0

    def initialize(faceplate:, x:, y:, style: :normal)
      super(faceplate: faceplate, x: x, y: y, diameter: DIAMETER)
      @slug = 'button'
      @slug += '-reversed' if style == :reversed
      @ring_color = faceplate.foreground
      @center_color = faceplate.background
      @ring_color, @center_color = @center_color, @ring_color if style == :reversed
    end

    def draw(svg:, x:, y:, state: :off)
      center_color = state == :on ? @center_color : @ring_color
      stroke_width = DIAMETER / 6.0
      circle_diameter = DIAMETER - stroke_width
      circle_radius = circle_diameter / 2.0
      svg.circle(cx: x, cy: y, r: circle_radius, 'stroke-width' => stroke_width, fill: center_color, stroke: @ring_color)
    end

    def svg_files
      [:on, :off].map do |state|
        position = state == :off ? 1 : 2
        path = faceplate.slug / "#{@slug}-#{position}"
        svg_file(path: path) do |svg|
          draw_control(svg: svg, state: state)
        end
      end
    end
  end
end
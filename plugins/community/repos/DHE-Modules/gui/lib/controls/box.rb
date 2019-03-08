require_relative '../dimensions'
require_relative '../control'

module DHE
  class Box < Control
    CORNER_RADIUS = 1.0
    BUFFER = PADDING + STROKE_INSET

    def initialize(faceplate:, top:, right:, bottom:, left:, style: :normal)
      super(faceplate: faceplate, top: top - BUFFER, right: right + BUFFER, bottom: bottom + BUFFER, left: left -
          BUFFER)
      @stroke = faceplate.foreground
      @fill = style == :normal ? faceplate.background : @stroke
    end

    def draw_faceplate(svg:)
      svg.rect(x: left, y: top, width: @width, height: @height,
               rx: CORNER_RADIUS, ry: CORNER_RADIUS,
               stroke: @stroke, fill: @fill, 'stroke-width' => STROKE_WIDTH)
    end
  end
end
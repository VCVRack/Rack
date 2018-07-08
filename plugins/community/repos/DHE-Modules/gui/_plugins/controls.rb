class Bounded
  attr_reader :top, :right, :bottom, :left

  def initialize(top:, right:, bottom:, left:)
    @top = top
    @right = right
    @bottom = bottom
    @left = left
  end

  def width
    @right - @left
  end

  def height
    @bottom - @top
  end

  def center
    OpenStruct.new(x: width / 2.0 + left, y: height / 2.0 + top)
  end

  def translate(delta_x: 0, delta_y: 0)
    @top += delta_y
    @right += delta_x
    @bottom += delta_y
    @left += delta_x
  end

  def move_center_to(x: center.x, y: center.y)
    delta_x = x - center.x
    delta_y = y - center.y
    translate(delta_x: delta_x, delta_y: delta_y)
  end
end

class Control < Bounded
  def initialize(x:, y:, width:, height: width)
    super(top: y - height / 2.0, right: x + width / 2.0, bottom: y + height / 2.0, left: x - width / 2.0)
  end
end

class RoundControl < Control
  attr_reader :diameter

  def initialize(x:, y:, diameter:)
    super(x: x, y: y, width: diameter)
    @diameter = diameter
  end

  def radius
    diameter / 2.0
  end
end

class ButtonControl < RoundControl
  DIAMETER = 6.0

  def initialize(x: 0.0, y: 0.0, style:, state:, dark:, light:)
    super(x: x, y: y, diameter: DIAMETER)
    @style = style.to_sym
    @state = state.to_sym
    @button_color = @style == :dark ? dark : light
    if @style == :dark
      @state_color = @state == :on ? light : dark
    else
      @state_color = @state == :on ? dark : light
    end
  end

  def name
    "button-#{@style}-#{@state}"
  end

  def align(padding, alignment, other)
    new_x = case alignment
              when :right_of
                other.right + padding + radius
              when :left_of
                other.left - padding - radius
              else
                center.x
            end
    move_center_to(x: new_x, y: other.center.y)
  end

  def svg
    stroke_width = diameter / 6.0
    circle_diameter = diameter - stroke_width
    circle_radius = circle_diameter / 2.0
    %Q[
      <circle cx="#{center.x}" cy="#{center.y}" r="#{circle_radius}" stroke-width="#{stroke_width}" fill="#{@state_color}" stroke="#{@button_color}"/>
    ]
  end
end

class KnobControl < RoundControl
  DIAMETER = 12.7

  def initialize(x: 0.0, y: 0.0, knob_color:, pointer_color:)
    super(x: x, y: y, diameter: 12.7)
    @knob_color = knob_color
    @pointer_color = pointer_color
  end

  def name
    'knob-large'
  end

  def svg
    pointer_width = radius / 8.0
    pointer_length = radius - pointer_width
    %Q[
      <g transform="translate(#{center.x} #{center.y})" stroke="#{@pointer_color}" fill="#{@knob_color }">
        <circle r="#{radius}" stroke="none"/>
        <line y2="-#{pointer_length}" stroke-width="#{pointer_width }" stroke-linecap="round"/>
      </g>
    ]
  end
end

class PortControl < RoundControl
  DIAMETER = 8.4

  def initialize(x: 0.0, y: 0.0, metal_color:, shadow_color:)
    super(x: x, y: y, diameter: 8.4)
    @metal_color = metal_color
    @shadow_color = shadow_color
  end

  def name
    'port'
  end

  def svg
    stroke_width = diameter * 0.025
    sleeve_diameter = diameter - stroke_width
    step = sleeve_diameter / 7.0
    sleeve_radius = sleeve_diameter / 2.0
    ring_radius = sleeve_radius - step
    tip_radius = ring_radius - step
    %Q[
    <g transform="translate(#{center.x} #{center.y})" stroke="#{@shadow_color}" fill="#{@metal_color}" stroke-width="#{stroke_width}">
      <circle r="#{sleeve_radius}"/>
      <circle r="#{ring_radius }"/>
      <circle r="#{tip_radius}" fill="#{@shadow_color}"/>
    </g>
    ]
  end
end

class SwitchControl < Control
  WIDTH = 3.0

  def initialize(x: 0.0, y: 0.0, positions:, state:, dark:, light:)
    super(x: x, y: y, width: WIDTH, height: positions * WIDTH)
    @positions = positions
    @state = state
    @dark = dark
    @light = light
    @position =
        case @state
          when :high
            1.0
          when :low
            -1.0
          else
            0.0
        end
  end

  def name
    "switch-#{@positions}-#{@state}"
  end

  def svg
    box_stroke_width = width / 8.0
    interior_inset = box_stroke_width / 2.0

    box_width = width - box_stroke_width
    box_height = height - box_stroke_width
    box_left = -width / 2.0 + interior_inset
    box_top = -height / 2.0 + interior_inset

    interior_width = box_width - box_stroke_width
    interior_height = box_height - box_stroke_width
    corner_radius = interior_inset

    knurl_stroke_width = 0.25
    knurl_inset = knurl_stroke_width * 2.0
    knurl_length = interior_width - knurl_inset
    knurl_left = knurl_length / -2.0
    knurl_right = knurl_left + knurl_length
    knurl_spacing = knurl_stroke_width * 2.0

    lever_height = knurl_spacing * 4.0 + knurl_stroke_width
    lever_inset = knurl_stroke_width
    lever_distance = (interior_height - lever_height) / 2.0 - lever_inset
    lever_offset = lever_distance * -@position
    lever = (-2..2)
                .map {|index| knurl_spacing * index + lever_offset}
                .map {|y| %Q[<line x1="#{knurl_left}" x2="#{knurl_right}" y1="#{y}" y2="#{y}" stroke-width="#{knurl_stroke_width}" stroke-linecap="round"/>]}
                .join("\n")
    %Q[
      <g transform="translate(#{center.x} #{center.y})" fill="#{@light}" stroke="#{@dark}">
        <rect x="#{box_left}" y="#{box_top}" width="#{box_width}" height="#{box_height}"
              rx="#{corner_radius}" ry="#{corner_radius}"
              stroke-width="#{box_stroke_width}"/>
        #{lever}
      </g>
    ]
  end
end

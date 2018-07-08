require 'ostruct'
require_relative 'page-color'
require_relative 'controls'

module PanelFilters
  include DHE::PageColor

  MM_PER_INCH = 25.4
  PX_PER_INCH = 75.0
  PX_PER_MM = PX_PER_INCH / MM_PER_INCH
  MM_PER_HP = 5.08

  STROKE_WIDTH = 0.35
  STROKE_INSET = STROKE_WIDTH / 2.0
  PADDING = 1.0

  PLUGIN_LABEL_INSET = 9.0
  PANEL_HEIGHT = 128.5

  PLUGIN_FONT = 12.0 / PX_PER_MM
  LARGE_FONT = 9.0 / PX_PER_MM
  SMALL_FONT = 7.0 / PX_PER_MM

  class Text
    ASCENT_RATIO = 2.0 / 3.0 # For Proxima Nova font

    def initialize(text:, size:, color:)
      @text = text
      @size = size
      @color = color
    end

    def ascent
      @size * ASCENT_RATIO
    end

    def descent
      @size - ascent
    end

    def svg(x:, y:, attributes:)
      %Q[<text #{attributes} fill="#{@color}" x="#{x}" y="#{y}" style="font-family:Proxima Nova;font-weight:bold;font-size:#{@size}px">#{@text}</text>]
    end
  end

  class Label < Bounded
    ALIGNMENT_ATTRIBUTES_TEMPLATE = %[dominant-baseline="%s" text-anchor="%s"]
    ALIGNMENT_ATTRIBUTES = {
        above: ALIGNMENT_ATTRIBUTES_TEMPLATE % %w{baseline middle},
        below: ALIGNMENT_ATTRIBUTES_TEMPLATE % %w{hanging middle},
        right: ALIGNMENT_ATTRIBUTES_TEMPLATE % %w{middle start},
    }

    def initialize(text, padding, alignment, control)
      @text = text
      @alignment = alignment
      case alignment
        when :above
          @x = control.center.x
          @y = control.top - padding
        when :below
          @x = control.center.x
          @y = control.bottom + padding
        when :right_of
          @x = control.right + padding
          @y = control.center.y
        else
          @x = control.center.x
          @y = control.center.y
      end
      super(top: @y - @text.ascent, right: @x, bottom: @y + @text.descent, left: @x)
    end

    def svg
      @text.svg(x: @x, y: @y, attributes: ALIGNMENT_ATTRIBUTES[@alignment])
    end
  end

  class Box < Bounded
    CORNER_RADIUS = 1.0
    BUFFER = PADDING + STROKE_INSET

    def initialize(content_bounds:, border_color:, background_color:)
      super(top: content_bounds.top - BUFFER, right: content_bounds.right + BUFFER, bottom: content_bounds.bottom + BUFFER, left: content_bounds.left - BUFFER)
      @border_color = border_color
      @background_color = background_color
    end

    def svg
      %Q[
      <rect x="#{left}" y="#{top}" width="#{width}" height="#{height}"
        stroke-width="#{STROKE_WIDTH}" rx="#{CORNER_RADIUS}" ry="#{CORNER_RADIUS}"
        stroke="#{@border_color}" fill="#{@background_color}"/>
      ]
    end

    def self.around(content:, border_color:, background_color:)
      content_bounds = Bounded.new(top: content.map(&:top).min, right: content.map(&:right).max, bottom: content.map(&:bottom).max, left: content.map(&:left).min)
      Box.new(content_bounds: content_bounds, border_color: border_color, background_color: background_color)
    end
  end

  def button(page, x, y, label)
    dark = dark(page)
    light = light(page)
    button = ButtonControl.new(x: x, y: y, style: :dark, state: :off, dark: dark, light: light)
    label_text = Text.new(text: label, color: dark, size: SMALL_FONT)
    items = [Label.new(label_text, PADDING, :above, button)]
    items << button if page['draw_controls']
    items.map(&:svg).join("\n")
  end

  def large_knob(page, x, y, label)
    dark = dark(page)
    light = light(page)
    knob = KnobControl.new(x: x, y: y, knob_color: dark, pointer_color: light)
    label_text = Text.new(text: label, color: dark, size: LARGE_FONT)
    items = [Label.new(label_text, PADDING, :above, knob)]
    items << knob if page['draw_controls']
    items.map(&:svg).join("\n")
  end

  def cv(page, x, y)
    dark = dark(page)
    light = light(page)
    draw = page['draw_controls']
    port = PortControl.new(x: x, y: y, metal_color: light, shadow_color: dark)
    label_text = Text.new(text: 'CV', color: dark, size: SMALL_FONT)
    items = [Label.new(label_text, PADDING, :above, port)]
    items << port if draw
    items.map(&:svg).join("\n")
  end

  def port_button(port_x:, port_y:, label:, button_position:, foreground_color:, background_color:, label_color:, metal_color:, shadow_color:, button_style:, draw:)
    port = PortControl.new(x: port_x, y: port_y, metal_color: metal_color, shadow_color: shadow_color)
    label_text = Text.new(text: label, color: label_color, size: SMALL_FONT)
    label = Label.new(label_text, PADDING, :above, port)
    button = ButtonControl.new(style: button_style, state: :off, dark: shadow_color, light: metal_color)
    button.align(PADDING, button_position, port)
    box = Box.around(content: [port, label, button], border_color: foreground_color, background_color: background_color)
    items = [box, label]
    items += [port, button] if draw
    items.map(&:svg).join("\n")
  end

  def in_port_button(page, x, y, label)
    dark = dark(page)
    light = light(page)
    draw = page['draw_controls']
    port_button(port_x: x, port_y: y, label: label, button_position: :right_of, foreground_color: dark, background_color: light, label_color: dark, metal_color: light, shadow_color: dark, button_style: :dark, draw: draw)
  end

  def out_port_button(page, x, y, label)
    dark = dark(page)
    light = light(page)
    draw = page['draw_controls']
    port_button(port_x: x, port_y: y, label: label, button_position: :left_of, foreground_color: dark, background_color: dark, label_color: light, metal_color: light, shadow_color: dark, button_style: :light, draw: draw)
  end

  def port(x:, y:, foreground_color:, background_color:, label:, label_color:, metal_color:, shadow_color:, draw:)
    port = PortControl.new(x: x, y: y, metal_color: metal_color, shadow_color: shadow_color)
    label_text = Text.new(text: label, color: label_color, size: SMALL_FONT)
    label = Label.new(label_text, PADDING, :above, port)
    box = Box.around(content: [port, label], border_color: foreground_color, background_color: background_color)
    items = [box, label]
    items << port if draw
    items.map(&:svg).join("\n")
  end

  def in_port(page, x, y, label)
    dark = dark(page)
    light = light(page)
    draw = page ['draw_controls']
    port(x: x, y: y, foreground_color: dark, background_color: light, label: label, label_color: dark, metal_color: light, shadow_color: dark, draw: draw)
  end

  def out_port(page, x, y, label)
    dark = dark(page)
    light = light(page)
    draw = page ['draw_controls']
    port(x: x, y: y, foreground_color: dark, background_color: dark, label: label, label_color: light, metal_color: light, shadow_color: dark, draw: draw)
  end

  def duration_switch(page, x, y)
    switch(page, x, y, :mid, '100', '1', '10')
  end

  def polarity_switch(page, x, y)
    switch(page, x, y, :high, 'UNI', 'BI',)
  end

  def shape_switch(page, x, y)
    switch(page, x, y, :low, 'S', 'J',)
  end

  def switch(page, x, y, position, high, low, mid = nil)
    dark = dark(page)
    light = light(page)
    draw = page['draw_controls']
    switch = SwitchControl.new(x: x, y: y, positions: mid ? 3 : 2, dark: dark, light: light, state: position.to_sym)
    high_text = Text.new(text: high, size: SMALL_FONT, color: dark)
    low_text = Text.new(text: low, size: SMALL_FONT, color: dark)
    items = [
        Label.new(high_text, PADDING + STROKE_INSET, :above, switch),
        Label.new(low_text, PADDING + STROKE_INSET, :below, switch),
    ]
    if mid
      mid_text = Text.new(text: mid, size: SMALL_FONT, color: dark)
      items << Label.new(mid_text, PADDING / 2.0 + STROKE_INSET, :right_of, switch)
    end
    items << switch if draw
    items.map(&:svg).join("\n")
  end

  def panel(page)
    dark = dark(page)
    light = light(page)
    panel = Bounded.new(top: 0.0, right: width(page), bottom: PANEL_HEIGHT, left: 0.0)
    name_label_text = Text.new(text: page['title'], color: dark, size: PLUGIN_FONT)
    name_label = Label.new(name_label_text, -PLUGIN_LABEL_INSET, :above, panel)
    author_label_text = Text.new(text: 'DHE', color: dark, size: PLUGIN_FONT)
    author_label = Label.new(author_label_text, -PLUGIN_LABEL_INSET, :below, panel)
    box = %Q[<rect x="#{panel.left}" y="#{panel.top}" width="#{panel.width}" height="#{panel.height}" stroke="#{dark}" fill="#{light}" stroke-width="1"/>]
    box + [name_label, author_label].map(&:svg).join("\n")
  end

  def connector(page, x1, y1, x2, y2)
    %Q[<line x1="#{x1}" y1="#{y1}" x2="#{x2}" y2="#{y2}" stroke="#{dark(page)}" stroke-width="#{STROKE_WIDTH}" />]
  end

  def width(page)
    hp_to_mm(page['width'])
  end

  def mm_to_px(mm)
    mm * PX_PER_MM
  end

  def hp_to_mm(hp)
    hp * MM_PER_HP
  end
end

Liquid::Template.register_filter(PanelFilters)

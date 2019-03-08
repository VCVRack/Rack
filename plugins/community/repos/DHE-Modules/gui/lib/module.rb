require 'color'

require_relative 'controls/button'
require_relative 'controls/counter'
require_relative 'controls/knob'
require_relative 'controls/label'
require_relative 'controls/line'
require_relative 'controls/port'
require_relative 'controls/toggle'
require_relative 'svg_file'

module DHE
  JSON_PARSING_OPTIONS = { symbol_keys: true }

  class Module
    attr_reader :name, :slug, :foreground, :background, :controls

    def initialize(name:, hp:, foreground:, background:)
      @name = name
      @slug = Pathname(@name.downcase.sub(' ', '-'))
      @width = hp * MM_PER_HP
      @width_px = @width * PX_PER_MM
      @height_px = PANEL_HEIGHT * PX_PER_MM

      @foreground = "##{Color::HSL.new(*foreground).to_rgb.hex}"
      @background = "##{Color::HSL.new(*background).to_rgb.hex}"
      @faceplate_items = [
          Label.new(faceplate: self, text: @name, size: :title, x: @width / 2, y: PANEL_LABEL_INSET),
          Label.new(faceplate: self, text: 'DHE', size: :title, alignment: :below, x: @width / 2, y: PANEL_HEIGHT - PANEL_LABEL_INSET)
      ]
      @controls = []
    end

    def faceplate_file
      SvgFile.new(path: slug, width: @width_px, height: @height_px, has_text: true) do |svg|
        svg.g(transform: "scale(#{PX_PER_MM})") do |g|
          g.rect(x: 0, y: 0, width: @width, height: PANEL_HEIGHT,
                 stroke: @foreground, fill: @background, 'stroke-width' => 1)
          @faceplate_items.each do |item|
            item.draw_faceplate(svg: g)
          end
        end
      end
    end

    def image_file
      SvgFile.new(path: slug, width: @width_px, height: @height_px, has_text: true) do |svg|
        svg.g(transform: "scale(#{PX_PER_MM})") do |g|
          g.rect(x: 0, y: 0, width: @width, height: PANEL_HEIGHT,
                 stroke: @foreground, fill: @background, 'stroke-width' => 1)
          @faceplate_items.each do |item|
            item.draw_faceplate(svg: g)
          end
          @controls.each do |control|
            control.draw_faceplate(svg: g)
          end
        end
      end
    end

    def control_files
      @controls.flat_map(&:svg_files)
    end

    def attenuverter(x:, y:)
      knob(x: x, y: y, size: :tiny, label: '<tspan font-size="larger">-&#160;&#160;+</tspan>', label_size: :large)
    end

    def button(x:, y:, label:)
      button = Button.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: label, size: :small, x: x, y: button.top - PADDING)
      @faceplate_items << label
      @controls << button
    end

    def connector(left:, right:, y:)
      @faceplate_items << Line.new(faceplate: self, x1: left, y1: y, x2: right, y2: y)
    end

    def counter(x:, y:, name:, labels:, selection: 1, enabled: true)
      counter = Counter.new(faceplate: self, x: x, y: y, name: name, labels: labels,
                            selection: selection, enabled: enabled)
      @controls << counter
    end

    def cv_port(x:, y:)
      port = Port.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: 'CV', size: :small, x: x, y: port.top - PADDING)
      @faceplate_items << label
      @controls << port
    end

    def duration_toggle(x:, y:)
      toggle(x: x, y: y, labels: %w(1 10 100), selection: 2)
    end

    def input_port(x:, y:, label: 'IN')
      port = Port.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: label, size: :small, x: x, y: port.top - PADDING)
      @faceplate_items << Box.new(faceplate: self, top: label.top, right: port.right, bottom: port.bottom, left: port.left)
      @faceplate_items << label
      @controls << port
    end

    def knob(x:, y:, label:, size:, label_size:)
      knob = Knob.new(faceplate: self, size: size, x: x, y: y)
      @controls << knob
      @faceplate_items << Label.new(faceplate: self, text: label, size: label_size, x: x, y: knob.top - PADDING)
    end

    def large_knob(x:, y:, label:)
      knob(x: x, y: y, size: :large, label: label, label_size: :large)
    end

    def medium_knob(x:, y:, label:)
      knob(x: x, y: y, size: :medium, label: label, label_size: :small)
    end

    def output_port(x:, y:, label: 'OUT')
      port = Port.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: label, size: :small, x: x, y: port.top - PADDING, style: :reversed)
      @faceplate_items << Box.new(faceplate: self, style: :reversed, top: label.top, right: port.right, bottom: port.bottom, left: port.left)
      @faceplate_items << label
      @controls << port
    end

    def polarity_toggle(x:, y:, selection: 1)
      toggle(x: x, y: y, labels: %w(BI UNI), selection: selection)
    end

    def separator(y:)
      @faceplate_items << Line.new(faceplate: self, x1: 0, y1: y, x2: @width, y2: y)
    end

    def shape_toggle(x:, y:)
      toggle(x: x, y: y, labels: %w(J S), selection: 1)
    end

    def small_knob(x:, y:, label:)
      knob(x: x, y: y, size: :small, label: label, label_size: :small)
    end

    def toggle(x:, y:, labels:, selection:)
      toggle = Toggle.new(faceplate: self, x: x, y: y, size: labels.size, selection: selection)
      @controls << toggle
      @faceplate_items << Label.new(faceplate: self, text: labels.first, size: :small,
                                    x: x, y: toggle.bottom + PADDING,
                                    alignment: :below)
      @faceplate_items << Label.new(faceplate: self, text: labels[1], size: :small,
                                    x: toggle.right + PADDING / 2.0, y: y,
                                    alignment: :right_of) if labels.size == 3
      @faceplate_items << Label.new(faceplate: self, text: labels.last, size: :small,
                                    x: x, y: toggle.top - PADDING,
                                    alignment: :above)
    end
  end
end
require_relative '../module'

module DHE
  class Tapers < DHE::Module
    def initialize
      super(name: 'TAPERS', hp: 9, foreground: [30, 100, 30], background: [30, 100, 97])

      left = @width / 5.0 + 1.0 / 3.0
      right = @width - left
      center = @width / 2.0

      y = 24.0
      delta_y = 16.0
      panel_buffer = 4.0
      separator_offset = 10.0

      2.times do |i|
        connector(left: left, right: right, y: y)
        cv_port(x: left, y: y)
        attenuverter(x: center, y: y)
        medium_knob(x: right, y: y, label: 'LEVEL')

        y += delta_y

        connector(left: left, right: right, y: y)
        cv_port(x: left, y: y)
        attenuverter(x: center, y: y)
        medium_knob(x: right, y: y, label: 'CURVE')

        y += delta_y

        shape_toggle(x: left, y: y)
        polarity_toggle(x: center, y: y)
        output_port(x: right, y: y)

        separator(y: y + separator_offset) if i == 0

        y += delta_y + panel_buffer
      end
    end
  end
end

MODULE_TO_FILENAME[DHE::Tapers.new] = __FILE__

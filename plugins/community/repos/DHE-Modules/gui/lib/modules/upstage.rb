require_relative '../module'

module DHE
  class Upstage < DHE::Module
    def initialize
      super(name: 'UPSTAGE', hp: 5, foreground: [210, 100, 30], background: [210, 100, 97])

      left = @width / 4.0 + 1.0 / 3.0
      right = @width - left
      center = @width / 2.0

      y = 25.0
      delta_y = 18.5

      large_knob(x: center, y: y, label: 'LEVEL')

      y += delta_y
      cv_port(x: left, y: y)
      polarity_toggle(x: right, y: y, selection: 2)

      y += delta_y
      button(x: left, y: y, label: 'WAIT')
      button(x: right, y: y, label: 'TRIG')

      y = 82.0
      delta_y = 15.0

      input_port(x: left, y: y, label: 'WAIT')

      y += delta_y
      input_port(x: left, y: y, label: 'TRIG')
      output_port(x: right, y: y, label: 'TRIG')

      y += delta_y
      output_port(x: right, y: y)
    end
  end
end

MODULE_TO_FILENAME[DHE::Upstage.new] = __FILE__

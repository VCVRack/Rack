require_relative '../module'

module DHE
  class Stage < DHE::Module
    def initialize
      super(name: 'STAGE', hp: 5, foreground: [120, 100, 30], background: [120, 100, 97])

      left = @width / 4.0 + 1.0 / 3.0
      right = @width - left
      center = @width / 2.0

      y = 25.0
      delta_y = 18.5

      large_knob(x: center, y: y, label: 'LEVEL')

      y += delta_y

      large_knob(x: center, y: y, label: 'CURVE')

      y += delta_y

      large_knob(x: center, y: y, label: 'DURATION')

      y = 82.0
      delta_y = 15.0

      input_port(x: left, y: y, label: 'DEFER')
      output_port(x: right, y: y, label: 'ACTIVE')

      y += delta_y

      input_port(x: left, y: y, label: 'TRIG')
      output_port(x: right, y: y, label: 'EOC')

      y += delta_y

      input_port(x: left, y: y)
      output_port(x: right, y: y)
    end
  end
end

MODULE_TO_FILENAME[DHE::Stage.new] = __FILE__

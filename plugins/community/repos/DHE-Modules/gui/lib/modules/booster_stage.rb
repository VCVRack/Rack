require_relative '../module'
require_relative '../controls/box'
require_relative '../controls/button'
require_relative '../controls/label'
require_relative '../controls/port'
require_relative '../dimensions'

module DHE
  class BoosterStage < DHE::Module
    def initialize
      super(name: 'BOOSTER STAGE', hp: 8, foreground: [0, 100, 30], background: [0, 100, 97])
      left = @width / 6.0 + 1.0 / 3.0
      right = @width - left
      center = @width / 2.0

      y = 25.0
      delta_y = 18.5

      connector(left: left, right: right, y: y)
      cv_port(x: left, y: y)
      large_knob(x: center, y: y, label: 'LEVEL')
      polarity_toggle(x: right, y: y)

      y += delta_y

      connector(left: left, right: right, y: y)
      cv_port(x: left, y: y)
      large_knob(x: center, y: y, label: 'CURVE')
      shape_toggle(x: right, y: y)

      y += delta_y

      connector(left: left, right: right, y: y)
      cv_port(x: left, y: y)
      large_knob(x: center, y: y, label: 'DURATION')
      duration_toggle(x: right, y: y)

      y = 82.0
      delta_y = 15.0

      input_button_port(x: left, y: y, label: 'DEFER')
      output_button_port(x: right, y: y, label: 'ACTIVE')

      y += delta_y

      input_button_port(x: left, y: y, label: 'TRIG')
      output_button_port(x: right, y: y, label: 'EOC')

      y += delta_y

      input_port(x: left, y: y)
      output_port(x: right, y: y)
    end

    def input_button_port(x:, y:, label:)
      port = Port.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: label, size: :small, x: x, y: port.top - PADDING)
      button_x = port.right + PADDING + Button::DIAMETER / 2.0
      button = Button.new(faceplate: self, x: button_x, y: y)
      @faceplate_items << Box.new(faceplate: self, top: label.top, right: button.right, bottom: port.bottom, left: port
                                                                                                                       .left)
      @faceplate_items << label
      @controls << port
      @controls << button
    end

    def output_button_port(x:, y:, label:)
      port = Port.new(faceplate: self, x: x, y: y)
      label = Label.new(faceplate: self, text: label, size: :small, x: x, y: port.top - PADDING, style: :reversed)
      button_x = port.left - PADDING - Button::DIAMETER / 2.0
      button = Button.new(faceplate: self, x: button_x, y: y, style: :reversed)
      @faceplate_items << Box.new(faceplate: self, style: :reversed, top: label.top, right: port.right, bottom: port
                                                                                                                    .bottom, left: button.left)
      @faceplate_items << label
      @controls << port
      @controls << button
    end
  end
end

MODULE_TO_FILENAME[DHE::BoosterStage.new] = __FILE__

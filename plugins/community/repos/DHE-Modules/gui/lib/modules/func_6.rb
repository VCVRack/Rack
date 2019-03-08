require_relative '../module'
module DHE
  class Func6 < DHE::Module
    def initialize
      super(name: 'FUNC 6', hp: 12, foreground: [220, 100, 40], background: [40, 50, 96])

      center = @width / 2.0
      left = @width / 7.0
      right = @width - left
      left_center = (left + center) / 2.0
      right_center = @width - left_center

      row_count = 6
      last_row = row_count - 1

      top = 23.0
      bottom = 108.0
      delta_y = (bottom - top) / last_row

      port_offset = 1.25

      y = top

      (0..last_row).each do |row|
        y = top + row * delta_y
        connector(left: left, right: right, y: y)
        input_port(x: left, y: y + port_offset)
        toggle(x: left_center, y: y, labels: %w(ADD MULT), selection: 1)
        large_knob(x: center, y: y, label: ' ')
        counter(x: right_center, y: y, name: 'add', labels: %w(0–5 ±5 0–10 ±10), selection: 2)
        counter(x: right_center, y: y, name: 'mult', labels: %w(0–1 ±1 0–2 ±2), selection: 2, enabled: false)
        output_port(x: right, y: y + port_offset)
      end
    end
  end
end

MODULE_TO_FILENAME[DHE::Func6.new] = __FILE__

require_relative '../module'
module DHE
  class Func < DHE::Module
    def initialize
      super(name: 'FUNC', hp: 3, foreground: [220, 100, 40], background: [40, 50, 96])

      center = @width / 2.0

      top = 23.0
      bottom = 108.0
      row_count = 6
      last_row = row_count - 1
      delta_y = (bottom - top) / last_row

      port_offset = 1.25

      y = top
      input_port(x: center, y: y + port_offset)

      y += delta_y
      toggle(x: center, y: y, labels: %w(ADD MULT), selection: 1)

      y += delta_y
      large_knob(x: center, y: y, label: ' ')

      y += delta_y
      counter(x: center, y: y, name: 'add', labels: %w(0–5 ±5 0–10 ±10), selection: 2)
      counter(x: center, y: y, name: 'mult', labels: %w(0–1 ±1 0–2 ±2), selection: 2, enabled: false)

      y += 2.0 * delta_y
      output_port(x: center, y: y + port_offset)
    end
  end
end

MODULE_TO_FILENAME[DHE::Func.new] = __FILE__

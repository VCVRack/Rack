module DHE
  module PageColor
    def hslcolor(page)
      Color::HSL.new(*page['color'])
    end

    def rgbhex(color)
      "##{color.to_rgb.hex}"
    end

    def light(page)
      hsl = hslcolor(page)
      hsl.l = 0.97
      rgbhex(hsl)
    end

    def dark(page)
      rgbhex(hslcolor(page))
    end
  end
end
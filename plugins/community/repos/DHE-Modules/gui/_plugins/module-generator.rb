require 'rake'
require 'color'
require_relative 'controls'
require_relative 'page-color'

module DHE
  SOURCE_DIR = '/panels'
  class PageWithoutAFile < Jekyll::Page
    def read_yaml(*)
      @data ||= {}
    end
  end

  class Generator < Jekyll::Generator
    include DHE::PageColor

    def generate(site)
      module_pages = site.pages.select {|page| page.url.start_with? SOURCE_DIR}
      module_pages.each do |module_page|
        site.pages += control_pages(module_page)
        site.pages << image_page(module_page)
      end
    end

    def control_page(module_page, control)
      page = PageWithoutAFile.new(module_page.site, __dir__, module_page.url.pathmap("%{^#{SOURCE_DIR},controls}X"), control.name.ext('svg'))
      page.data['layout'] = 'control'
      page.data['width'] = control.width
      page.data['height'] = control.height
      page.content = control.svg
      page
    end

    def control_pages(module_page)
      controls(module_page).map {|control| control_page(module_page, control)}
    end

    def controls(page)
      page.data['controls'].flat_map {|type, variants| send(type, page, variants)}
    end

    def image_page(module_page)
      page = PageWithoutAFile.new(module_page.site, __dir__, module_page.url.pathmap("%{^#{SOURCE_DIR},images}d"), module_page.name)
      page.data.merge!(module_page.data)
      page.data['draw_controls'] = true
      page.data['dark'] = dark(page)
      page.data['light'] = light(page)
      page.content = module_page.content
      page
    end

    def buttons(page, variants)
      variants.flat_map do |style|
        [:off, :on].map do |state|
          ButtonControl.new(style: style, state: state, dark: dark(page), light: light(page))
        end
      end
    end

    def ports(page, _)
      PortControl.new(metal_color: light(page), shadow_color: dark(page))
    end

    def knobs(page, _)
      KnobControl.new(knob_color: dark(page), pointer_color: light(page))
    end

    def switches(page, variants)
      variants.flat_map do |positions|
        states = [:high, :low]
        states << :mid if positions == 3
        states.map do |state|
          SwitchControl.new(positions: positions, state: state, dark: dark(page), light: light(page))
        end
      end
    end
  end
end
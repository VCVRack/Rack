require 'builder'

module DHE
  class SvgFile
    attr_reader :has_text, :path, :content

    def initialize(path:, width:, height:, has_text: false, **options)
      @path = path.sub_ext('.svg')
      @has_text = has_text
      @content = Builder::XmlMarkup.new(indent: 2).svg(version: "1.1", xmlns: "http://www.w3.org/2000/svg", width: width, height: height, **options) {|svg| yield(svg)}
    end

    def write(dir)
      file_path = dir / path
      file_path.parent.mkpath
      file_path.open('w') {|file| file.write @content}
    end
  end
end

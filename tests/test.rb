require 'rubygems'
require 'semacode'

def show_as_text(semacode)
  show = "\n"
  for row in semacode.data
    show += "\t"
    for column in row
      if column
        show += "1"
      else
        show += "0"
      end
    end
    show += "\n"
  end
  show += "\n"
end

def show_as_html(cell_size, prefix, semacode)
  show = ""
  css =<<CSS
  .row { clear: both; }
  .black
  {
  	background: black;
  	color: white;
  	width: #{cell_size}px;
  	height: #{cell_size}px;
  	padding: 0px;
  	margin: 0px;
  	border: 0px;    	
  	float: left;
  	display: block;
  }

  .white
  {
  	background: white;
  	color: black;
  	width: #{cell_size}px;
  	height: #{cell_size}px;
  	padding: 0px;
  	margin: 0px;
  	border: 0px;
  	display: block;
  	float: left;
  }
CSS
  show += "<html><head><style>\n<!-- \n#{css} \n-->\n</style><body>"
  show += "<pre>#{prefix}</pre>"
  for row in semacode.data
    show += "\t<div class='row'>\n"
    for column in row
      if column
        show += "\t\t<div class='black'></div>\n"
      else
        show += "\t\t<div class='white'></div>\n"
      end
    end
    show += "\t</div>\n"
  end
  show += "</body></html>\n"
end

def prelude
  show = "\nSemacodes in Ruby\n\n"

  semacode = DataMatrix::Encoder.new("http://sohne.net/projects/semafox")

  show += "width #{semacode.width}\n"
  show += "height #{semacode.height}\n"
  show += "raw_encoded_length #{semacode.raw_encoded_length}\n"
  show += "symbol_size #{semacode.symbol_size}\n"
  show += "ecc_bytes #{semacode.ecc_bytes}\n"


  show += "a text representation of a semacode\n\n"

  text = show + show_as_text(semacode)

  semacode.encode("http://sohne.net/")

  html = show_as_html(8, text, semacode)
end

semacode = DataMatrix::Encoder.new "http://www.ruby-lang.org"
puts show_as_text(semacode)
# comment line above and uncomment next line 
# to generate HTML instead of plain text
# puts prelude
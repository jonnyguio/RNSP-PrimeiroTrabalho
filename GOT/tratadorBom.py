import re


#reload(sys)
#sys.setdefaultencoding('cp860')

#abre o arquivo
arquivo = open('dados/FrasesSelecionadas.txt')

#le o arquivo
raw=arquivo.read()


raw=raw.replace(b'\xc3', b' ')
raw=raw.replace(b'\xc2\xa0', b' ')
raw=raw.replace(b'\xe2\x80\x93', b', ')
raw=raw.replace(b'\xc3\xaf', b'i')
raw=raw.replace(b'\xc4\x87', b'c')
raw=raw.replace(b'\xe2\x80\x8b', b' ')
raw=raw.replace(b'\xc3\xa7', b'c')
raw=raw.replace(b'\xc3\xa1', b'a')
raw=raw.replace(b'\xe2\x80\x94', b' - ')
raw=raw.replace(b'\xe2\x80\x95', b', ')
raw=raw.replace(b'\xe2\x80\x99', b"'")
raw=raw.replace(b'\xe2\x80\x98', b"'")
raw=raw.replace(b'\xc3\xbe', b'b')
raw=raw.replace(b'\xc3\xba', b'u')
raw=raw.replace(b'\xc3\xad', b'i')
raw=raw.replace(b'\xc3\xb3', b'o')
raw=raw.replace(b'\xc3\xb6', b'o')
raw=raw.replace(b'\xc3\x9e', b'p')
raw=raw.replace(b'\xc3\xbd', b'y')
raw=raw.replace(b'\xc3\xb8', b'o')
raw=raw.replace(b'\xe2\x80\x9c', b'"')
raw=raw.replace(b'\xe2\x80\x9d', b'"')
raw=raw.replace(b'\xc3\xa6', b'ae')
raw=raw.replace(b'\xe2', b'')
raw=raw.replace(b'\x80', b'')




print raw


import nltk
import re
from nltk.stem.snowball import SnowballStemmer
from nltk.corpus import stopwords


def identificaTitulo(cedula, tituloAnterior):
	titulo=cedula[0][1:len(cedula[0])]
	#fiz esse tamanho minimo para ele nao pegar nem um lixo, e como o menor nome de capitulo eh JON, nao teremos problema
	if titulo.upper()==titulo and len(titulo)>2:
		return cedula[0]
	else:
		return tituloAnterior

def melhoraListaFrases(listaFrasesRuim):
	listaFrasesBoa = []
	for frase in listaFrasesRuim:
		for fraseBoa in frase.split('\n'):
			if fraseBoa != '':
				#if fraseBoa[-1] != '.':
					#fraseBoa += '.'		#coloca ponto final onde estiver faltando
				listaFrasesBoa.append(fraseBoa)
	return listaFrasesBoa

def criaListaTags (listaFrasesBoa):
	#obtem os tokens, separados por frase
	tokensPorFrase = []
	for frase in listaFrasesBoa:
		tokensPorFrase.append(nltk.word_tokenize(frase))

	#gera uma lista com todos os tokens do texto
	tokensGeral = []
	for frase in tokensPorFrase:
		for token in frase:
			tokensGeral.append(token)

	#obtem a classificacao gramatica das palavras
	listaTagsRuim = nltk.pos_tag(tokensGeral)

	#gera uma lista de classificacoes mais facil de mexer
	listaTagsBoa = []
	for par in listaTagsRuim:
		listaTagsBoa.append([str(par[0]),str(par[1])])
	return listaTagsBoa

def GeraVetorDaRNSP(matrizFull, nomeDoArquivoTre, nomeDoArquivoTes):
	listaPalavrasUnicas=[]
	matrizDeEntrada=[]

	#constroi a wordlist do texto (usando o primitivo das palavras)
	for frase in matrizFull:
		for palavra in frase:
			if palavra not in listaPalavrasUnicas:
				listaPalavrasUnicas.append(palavra)

	#print listaPalavrasUnicas

    #constroi a matriz de entrada da RNSP (binaria)
	for frase in matrizFull:
		palavrasNaFrase=[0]*len(listaPalavrasUnicas)
		for palavra in frase:
			index=listaPalavrasUnicas.index(palavra)
			palavrasNaFrase[index]=1
		matrizDeEntrada.append(palavrasNaFrase)

	##DEBUGER
	# for frase in matrizDeEntrada:
	# 	print frase

###TENHO Q VER COMO ESCREVER A MATRIZ EM ARQUIVO E DONEEEEE
    #entrar com esses dados na RNSP em C++ -> como eu devo montar o arquivo para darmos de entrada?
    #90% sera usada para treino
    #10% sera usada para teste
	qtdFrases=len(matrizDeEntrada)
	#print qtdFrases

	limite=int(0.9*qtdFrases)
	#print limite

	cont=0
	while cont<limite:
		print matrizDeEntrada[cont]
		cont+=1

    # #gera arquivo com listas binarias dos dados de treino (falta criar o arquivo com as labels)
	# treino=open(nomeDoArquivoTre,'w')
	# cont=0
	# while cont<limite:
	# 	treino.write(matrizDeEntrada[cont])
	# 	cont+=1
	# treino.close()
	#
    # #gera arquivo com listas binarias dos dados de teste (falta criar o arquivo com as labels)
	# teste=open(nomeDoArquivoTes,'w')
	# while cont<qtdFrases:
	# 	teste.write(matrizDeEntrada[cont])
	# 	cont+=1
	# teste.close()

def GeraLabels(capCompleto, frasesSelecionadas):
	vetorLabels=[]
	for frase in capCompleto:
		if frase in frasesSelecionadas:
			vetorLabels.append('1')
		else:
			vetorLabels.append('0')
	#print vetorLabels



#usar NLTK para tokenizar o testo, retirar stopwords e fazer Stemming


############MAIN##############
#abre o arquivo
#arquivo = open('base de dados concatenada.txt')
arquivoEntrada = open('dados/TodasAsFrases.txt')

#le o arquivo
raw=arquivoEntrada.read()

#obtem o segmentador de sentencas
sent_tokenizer=nltk.data.load("tokenizers/punkt/english.pickle")

#separa as frases do texto
listaFrasesRuim = sent_tokenizer.tokenize(raw)

#melhora a lista de frases
listaFrasesBoa = melhoraListaFrases(listaFrasesRuim)

#cria a lista de duplas com classificacao das palavras
listaTagsBoa=criaListaTags(listaFrasesBoa)


#troca as labels de algumas palavras para seguir regras parecidas com o outro programa

prepLugarList=['above', 'across', 'after', 'against', 'along', 'among',
			'around', 'at', 'behind', 'below', 'beside', 'between', 'beyond the',
			'close to', 'down', 'from', 'in front of',
			'inside', 'in', 'into', 'near', 'next to', 'off', 'on',
			'onto', 'opposite', 'out of', 'outside', 'over',
			'past', 'round', 'through', 'to', 'towards', 'under', 'up']
familialist=['of the', 'from the']
blacklist=['in','on','at', 'beyond', 'as', '[',']', 'and', 'are', 'for', 'from','while','with',
			'under', 'to', 'does','into','is','again','angrily',
			'was','then','that','during','tells','takes','calls',
			'back','if','before','through','by','about','atop', 'until', 'since']
pessoaInicio = ['king', 'queen', 'prince', 'princess', 'lord', 'lady', 'ser', 'commander', 'Young']

cont=0
while cont < len(listaTagsBoa):
	if listaTagsBoa[cont][1]=='NNP' or listaTagsBoa[cont][1]=='NNPS':
		listaTagsBoa[cont][1]='ENOMEADA'

	#estou fazendo isso pois no final de cada frase de personagem temos ',' ao infez de um '.'
	elif listaTagsBoa[cont][1]==',':
		if (cont < (len(listaTagsBoa)-1)) and (listaTagsBoa[cont+1][1]=="''"): #essas aspas "''" aparecem como fecha aspas no formato pdf
			listaTagsBoa[cont][1]='PONTOFALA'
	cont+=1

for word in listaTagsBoa:
	if (word[0].lower() in blacklist) or (word[0].lower() in prepLugarList) or (word[0].lower() in familialist):
		word[1]='LNEGRA'
	if (word[0].lower() in pessoaInicio):
		word[1]='TITULO'


#gera a matriz de frases
fraseTemp = []
matrizDeFrases = []

cont=0
titulo=''
while cont < len(listaTagsBoa):
	titulo=identificaTitulo(listaTagsBoa[cont], titulo)
	#pegarei de frase em frase para analisar
	if listaTagsBoa[cont][1] != '.' and listaTagsBoa[cont][0]!=titulo:
		fraseTemp.append(listaTagsBoa[cont])
	else:
		if listaTagsBoa[cont][0]!=titulo:
			fraseTemp.append(listaTagsBoa[cont])
		#isso ira fazer com que todas as falas sejam classificadas do inicio o " ate o fim do "
		if cont<(len(listaTagsBoa)-1) and listaTagsBoa[cont+1][1]=="''":
			cont+=1
			fraseTemp.append(listaTagsBoa[cont])
		#se entrar aqui significa que eh um fim de frase, e o programa ira pontuar, para saber se a frase eh um modelo de sumario
		matrizDeFrases.append(fraseTemp)

		fraseTemp=[]
	cont+=1


#GERA O ARQUIVO DE LABELS
frasesSelecionadas = open('dados/FrasesSelecionadas.txt').read()
vec=frasesSelecionadas.split('\n')



# #USEI ISSO PARA FORMATAR O ARQUIVO ORIGINAL
# oi=sent_tokenizer.tokenize(frasesSelecionadas)
# tokensPorFrase = []
# for frase in oi:
# 	tokensPorFrase.append(nltk.word_tokenize(frase))
# for frase in tokensPorFrase:
# 	for el in frase:
# 		print el,
# 	print

todasAsFrases = open('dados/TodasAsFrases.txt').read()
vecT=todasAsFrases.split('\n')
#the last element is null, so i will remove
del vecT[-1]

GeraLabels(vecT,vec)

#steaming e retirada de stopwords
stemmer = SnowballStemmer("english")
stop = set(stopwords.words('english'))
myStopList = [ '``', ',', "''", '.' ]

matrizDeFrasesFinalTag = []
matrizDeFrasesFinalPalavra = []

for frase in matrizDeFrases:
	#ira compor a matriz de tags para a entrada na RNSP
	fraseSemStopTag=[]
	#ira compor a matriz de palavras para a entrada na RNSP
	fraseSemStopPalavra=[]
	for elemento in frase:
		elemento[0]=stemmer.stem(elemento[0])
		if (elemento[0] not in stop) and (elemento[1] not in myStopList):
			fraseSemStopTag.append(elemento[1])
			fraseSemStopPalavra.append(elemento[0])
	#essa sera a matriz que sera binarizada com tags
	matrizDeFrasesFinalTag.append(fraseSemStopTag)
	#essa sera a matriz que sera binarizada com palavras
	matrizDeFrasesFinalPalavra.append(fraseSemStopPalavra)


###DEBUGER###
# for frase in matrizDeFrasesFinalPalavra:
# 	for elemento in frase:
# 		print elemento,
# 	print

GeraVetorDaRNSP(matrizDeFrasesFinalPalavra, '/home/rgaio/Desktop/treino-palavras', '/home/rgaio/Desktop/teste-palavras')
GeraVetorDaRNSP(matrizDeFrasesFinalTag, '/home/rgaio/Desktop/treino-tags', '/home/rgaio/Desktop/teste-tags')

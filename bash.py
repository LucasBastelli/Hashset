import subprocess
import re
import matplotlib.pyplot as plt
import sys
import numpy
from scipy.stats import bootstrap


def rodar(arq,rep,Tamanho,update,programa):
    aux=1
    while(aux<=rep):			#Quantidade de repeticoes
        print("Repetição "+str(aux)+" de "+str(rep)) 		#Variados tamanhos
        teste=subprocess.run(programa, stdout=subprocess.PIPE)
        linhas = teste.stdout.splitlines()
        salvarRAW(arq,'65536',aux,linhas)
        aux+=1	
    return


def criaIndice(arq,rep,Tamanho):
	arquivo=open(arq+"/indice.txt","w")			#Salva os tamanhos e quantidade de testes
	arquivo.write("["+str(rep)+","+str(Tamanho)+']')
	return

def abrirIndice(arq):
	arquivo=open(arq+"/indice.txt","r")		#Resgata os tamanhos e quantidade de testes
	lista=arquivo.read()
	lista=eval(lista)
	return lista


def mkdir(arq):				#Cria o diretorio
	aux=1
	teste=subprocess.run(["mkdir",arq],stdout=subprocess.PIPE)
	return arq

def salvarRAW(diretorio,size,num,text):
	arq=open(diretorio+"/teste"+str(size)+"_"+str(num),"w")	#Nome do arquivo
	for line in text:
		arq.write(str(line)+"\n")				#Salva as linhas
	return

def abrirRAW(txt,diretorio,rep,indice):
	listaAux=[]
	lista=[]
	listaTam=[]
	contador=0
	contador2=1
	contadorTam=0
	tamanho=indice[0]
	while(contador<(len(indice))):
		contador2=1
		tamanho=indice[contadorTam]
		listaAux=[]
		while(contador2<=rep):
			arq=open(diretorio+"/teste"+str(tamanho)+"_"+str(contador2),"r")
			aux=arq.read()
			aux=aux.splitlines()
			listaAux.append(procura(txt,aux))
			contador2+=1
		listaTam.append(tamanho)
		lista.append(listaAux)		
		contadorTam+=1
		contador+=1
	return listaTam,lista

def enumera(lista):					#Remove os textos,deixando apenas os numeros
	retornar=[]
	if(type(lista) is list):
		for line in lista:
			valor=''
			line=str(line)
			line=re.sub(r'#txs          : ','',line)
			line=re.sub(r' / s','',line)
			line=list(line)
			x=0
			while(line[x]!='('):
				x+=1
			x+=1
			while(line[x]!=')'):
				valor=valor+line[x]
				x+=1
			retornar.append(eval(valor))
	else:
		valor=''
		lista=str(lista)
		lista=re.sub(r'#txs          : ','',lista)
		lista=re.sub(r' / s','',lista)
		lista=list(lista)
		x=0
		while(lista[x]!='('):
			x+=1
		x+=1
		while(lista[x]!=')'):
			valor=valor+lista[x]
			x+=1
		retornar=eval(valor)
		
	return retornar


def main(arg):
	rep=10					#Quantas vezes ira repetir
	tamanho=[65536]
	update=50				#Taxa de update
	PM=0
	programa="./hash"
	arq="/home/lucas/TCC/Testeshs/test"
	if(PM==1):
		programa=programa+"-pm"
		arq=arq+"PM"
	arq2=arq+"PM"	
	arq=arq+"_update"+str(update)
	arq2=arq2+"_update"+str(update)
	txt='#txs          :'
	arq=mkdir(arq)
	x=[]
	final=[]
	intervalo=[]
	if(len(arg)==1 or len(arg)>7):
		print("Modo de uso: python3 bash.py <argumento>\nDigite <python3 bash.py h> para lista de comandos")
		return
	
	elif(arg[1]=='h'):
		print("Comandos:\ng --> Plota o grafico com os dados prontos\na --> Roda o programa e produz o grafico em seguida\nr --> Apenas roda o programa")
		return
		
	elif(arg[1]=='a'):
		criaIndice(arq,rep,tamanho)
		rodar(arq,rep,tamanho,update,programa)
		x,y=abrirRAW(txt,arq,rep,tamanho)
		medias=mediaLista(y)
		for lista in y:
			intervalo.append(IntConfianca(lista))

		graf(arq,x,medias,intervalo)
		return
	
	elif(arg[1]=='r'):
		criaIndice(arq,rep,tamanho)
		rodar(arq,rep,tamanho,update,programa)
		return		
	
	elif(arg[1]=='g'):
		indice=abrirIndice(arq)
		rep=indice[0]
		indice.pop(0)
		indice=indice[0]
		x,y=abrirRAW(txt,arq,rep,indice)
		medias=mediaLista(y)
		for lista in y:
			intervalo.append(IntConfianca(lista))
		graf(arq,x,medias,intervalo)
		return
	
	elif(arg[1]=='c'):
		arq3="/home/lucas/IniCientifica/TestesllHD/testPM_update"+str(update)
		intervalo=[]
		intervalo2=[]
		intervalo3=[]
		indice=abrirIndice(arq)
		indice2=abrirIndice(arq2)
		indice3=abrirIndice(arq3)
		rep=indice[0]
		indice.pop(0)
		indice=indice[0]
		rep2=indice2[0]
		indice2.pop(0)
		indice2=indice2[0]
		rep3=indice3[0]
		indice3.pop(0)
		indice3=indice3[0]
		x,y=abrirRAW(txt,arq,rep,indice)
		x2,y2=abrirRAW(txt,arq2,rep2,indice2)
		x3,y3=abrirRAW(txt,arq3,rep3,indice3)
		medias=mediaLista(y)
		medias2=mediaLista(y2)
		medias3=mediaLista(y3)
		#GrafSpeedup(arq,x,medias,medias2,medias3)#y1=Ram,y2=PM,y3=ssd
		for lista in y:
			data = (lista,)
			#intervalo.append(IntConfianca(lista))
			var=(bootstrap(data, numpy.std, confidence_level=0.95))
			intervalo.append([var.confidence_interval[1],var.confidence_interval[0]])
		for lista in y2:
			#intervalo2.append(IntConfianca(lista))
			var=(bootstrap(data, numpy.std, confidence_level=0.95))
			intervalo2.append([var.confidence_interval[1],var.confidence_interval[0]])
		for lista in y3:
			data = (lista,)
			#intervalo3.append(IntConfianca(lista))
			var=(bootstrap(data, numpy.std, confidence_level=0.95))
			intervalo3.append([var.confidence_interval[1],var.confidence_interval[0]])
		graf_comparacao(arq,x,medias,medias2,medias3,numpy.array(intervalo),numpy.array(intervalo2),numpy.array(intervalo3))
		return
		
	else:
		print("Comando errado\nDigite <python3 bash.py h> para lista de comandos")
		return
	
	
#teste
main(sys.argv)
#criaIndice("/home/lucas/Desktop/teste",10,128,8)
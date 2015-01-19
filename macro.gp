#Macro per graficare dati su gnuplot con fit

#Inizializzazione

reset
set output #chiude eventuali files aperti
set terminal x11 enhanced
set fit errorvariables # Store errors in variables with name ???_err

#Definizione e formattazione del grafico

set title 'Pool 2D graph'
set xlabel 'Leech'	# {/Symbol roba_da_scrivere_in_greco_grazie_a_enhanced}
set ylabel 'Bbound'
set xrange [*:*]
set yrange [*:*]
unset key #toglie la legenda al grafico
#set samples 10000
#set logscale xy
#set xtics (80,90,100,200,300)
#set ytics (2000,4000,6000,8000,10000,20000)

#Fit normale
#f(x)=a*x+b
#a=10
#b=-1
#fit [x=*:*] f(x) "dati_discr1" using 1:2:3 via  a,b #x:y:Dy

#Fit retta 1 (alla bode)

#f1(x)=10**b1*x**a1
#a1=0
#b1=1.1
#fit [x=9000:80000] f1(x) "dati_" using 1:($5/$3):(sqrt(($4/$3)**2+($6/$5)**2)*$5/$3) via a1,b1

#Fit retta 2 (alla bode)

#f2(x)=10**b2*x**a2
#a2=1
#b2=1
#fit [x=*:1500] f2(x) "dati_" using 1:($5/$3):(sqrt(($4/$3)**2+($6/$5)**2)*$5/$3) via a2, b2

#Fit retta 3 (alla bode)

#f3(x)=10**b3*x**a3
#a3=-0.95
#b3=5.68
#fit [x=150000:*] f3(x) "dati_" using 1:($5/$3):(sqrt(($4/$3)**2+($6/$5)**2)*$5/$3) via a3,b3

#Fit retta 1

#f1(x)=a1*x+b1
#a1=1
#b1=0.5
#fit [x=-350:*] f1(x) "dati_" using 1:5:6 via a1,b1

#Fit retta 2
#f2(x)=a2*x+b2
#a2=1
#b2=1
#fit [x=*:50] f2(x) "dati_" using 5:1:2 via a2,b2
#f_2(x)=(x-b2)/a2


#Plot (scegliere le barre d'errore volute)
plot 'data_table_mgfast2262144' using 2:3:1 pointtype 13 pointsize 1 linetype palette

# Plot del log (Bode)
#plot 'dati_' using 1:(20*log10($5/$3)):2:((20/log(10))*sqrt(($4/$3)**2+($6/$5)**2)) with xyerrorbars pointtype 6 pointsize 0.1, 20*log10(f1(x)), 20*log10(f2(x)), 20*log10(f3(x))#, 20*log10(f(x)) #linecolor rgb "blue" linewidth 2


# Sacrifici a Cthulhu (incroci rette)

#print "\nfit_param,		Dfit_param,		err%"
#print a1, "\t", a1_err, "\t", 100*abs(a1_err/a1), "%"
#print b1, "\t", b1_err, "\t", 100*abs(b1_err/b1), "%"
#print a2, "\t", a2_err, "\t", 100*abs(a2_err/a2), "%"
#print a2, "\t", 0, 		"\t", 					0, "%"
#print b2, "\t", b2_err, "\t", 100*abs(b2_err/b2), "%"
#print a3, "\t", a3_err, "\t", 100*abs(a3_err/a3), "%"
#print b3, "\t", b3_err, "\t", 100*abs(b3_err/b3), "%"

# Bode-incontro (NOTA: la retta di mezzo è una costante)

#f1=10**((b2-b1)/(a1-a2))
#c1=-0.987
#print "\n"
#print "f1 = ", f1
#pezzo1=((b1-b2)*a2_err/a2)**2
#pezzo2=b2_err**2+b1_err**2
#pezzo3=2*((b1-b2)/a2)*c1*a2_err*b2_err
#print "Df1 = ", sqrt(pezzo1+pezzo2+pezzo3)*(log(10)/abs(a2))*f1

#f1=10**((b3-b1)/(a1-a3))
#c1=-0.999
#print "\n"
#print "f2 = ", f1
#pezzo1=((b1-b3)*a3_err/a3)**2
#pezzo2=b3_err**2+b1_err**2
#pezzo3=2*((b1-b3)/a3)*c1*a3_err*b3_err
#print "Df2 = ", sqrt(pezzo1+pezzo2+pezzo3)*(log(10)/abs(a3))*f1

#f2=10**((b2-b3)/(a3-a2))
#c2=-0.999
#print "f2 = ", f2
#pezzo1=((b2-b3)*a3_err/a3)**2
#pezzo2=b3_err**2+b2_err**2
#pezzo3=2*((b2-b3)/a3)*c2*a3_err*b3_err
#print "Df1 = ", sqrt(pezzo1+pezzo2+pezzo3)*(log(10)/abs(a3))*f2

#Rette-incontro (correl.)

#c1=-0.900
#c2=-0.982
#print "\nx0 = ", (b1-b2)/(a2-a1)
#pezzo1=((b1-b2)**2/(a1-a2)**4)*(a1_err**2+a2_err**2)
#pezzo2=(1/(a1-a2)**2)*(b1_err**2+b2_err**2)
#pezzo3=2*((b1-b2)/(a1-a2)**3)*(a1_err*b1_err*c1+a2_err*b2_err*c2)
#print "Dx0= ", sqrt(pezzo1+pezzo2+pezzo3)


#Ritorno all'output su schermo

set output #chiude il file pdf 
set terminal x11 enhanced #ritorna alla visualizzazione su schermo


#Funzioni per filtri PA e PB
#A1(f)=1/(1+{0,1}*f/f1)
#A2(f)=({0,1}*f/f2)/(1+{0,1}*f/f2)
#At(f)=A1(f)*A2(f)/(1+r*A1(f)*A2(f))

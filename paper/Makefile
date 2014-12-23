all: sat.pdf

clean:
	rm -f *.aux *.bbl *.dvi *.log stmp-* *.dat

bit20.dat: bit20.csv reducedata.py
	./reducedata.py < bit20.csv > bit20.dat

bit20.tex: bit20.dat bit20.plot
	gnuplot bit20.plot

bit16.dat: bit16.csv reducedata.py
	./reducedata.py < bit16.csv > bit16.dat

bit16.tex: bit16.dat bit16.plot
	gnuplot bit16.plot

results.dat: results.csv reducedata.py
	./reducedata.py < results.csv > results.dat

results.tex: results.dat results.plot
	gnuplot results.plot


stmp-latex-1: sat.tex results.tex bit16.tex bit20.tex
	latex sat
	touch stmp-latex-1

stmp-bibtex: stmp-latex-1 sat.bib
	bibtex sat
	touch stmp-bibtex

stmp-latex-2: stmp-bibtex
	latex sat
	touch stmp-latex-2

stmp-latex-3: stmp-latex-2
	latex sat
	touch stmp-latex-3

sat.pdf: stmp-latex-3
	dvipdf sat


%
% memoria.cls
% (c) 2014  M. Blanc
%
\NeedsTeXFormat{LaTeX2e}


\def\filename{memoriaeps}
\def\fileversion{v1.0}
\def\filedate{2014/03/15}

\ProvidesClass{memoria}[\filedate \fileversion Clase para memorias]

% Passes and class options to the underlying article class
\LoadClass[a4paper, 12pt]{article}
%\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
%\ProcessOptions

\ProvidesPackage{\filename}[\filedate]
\typeout{Package `\filename', \fileversion, <\filedate>}


% UTF8 + Español
\RequirePackage[utf8]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage[spanish]{babel}

% Estilos
\RequirePackage{xcolor}
\RequirePackage{titlesec}
\RequirePackage{graphicx}
\RequirePackage{a4wide}
\RequirePackage{parskip}

% Codigo & unidades
\RequirePackage{listings}
\RequirePackage{underscore}
\RequirePackage[binary-units]{siunitx}

% Tablas
\RequirePackage{subcaption}
\RequirePackage{tabu}
\RequirePackage{booktabs}

% Misc
\RequirePackage[colorlinks=true,linkcolor=black,urlcolor=black]{hyperref}
\RequirePackage{environ}

% Datos sobre el articulo
\newcommand{\asignatura 	}[1]{\def\myasignatura{#1}}
\newcommand{\curso      	}[1]{\def\mycurso{#1}}
\newcommand{\practica   	}[2]{\def\mynumpractica{#1} \def\mypractica{#2}}
\newcommand{\autores    	}[1]{\def\myautores{#1}}
\newcommand{\universidad	}[1]{\def\myuniversidad{#1}}

\title{\mypractica \\ \myasignatura}
\author{\myautores}

% Para graficos
\graphicspath{ {img/} }
\newcommand{\imagen}[2][1]{\includegraphics[width=#1\textwidth]{#2}}
\newenvironment{galeria}{
	\renewcommand{\imagen}[2][1.0]{
		\begin{subfigure}{##1\textwidth}
			\includegraphics[width=\textwidth]{##2}
		\end{subfigure}% eat newline
	}

	\begin{figure}[h!]
	\centering
}
{
	\end{figure}
}

% Y para tablas
\newenvironment{grupotablas}{%
	\NewEnviron{tabla}[2][]{%
		\begin{subtable}{1\textwidth}
			\centering
			\begin{tabu}{##2}
				\toprule
				\BODY
				\bottomrule
			\end{tabu}
			\caption{##1}
		\end{subtable}
	}%
	\begin{table}[h]
}{%
	\end{table}
}

% Utiles de tablas
\newcommand{\cabeceratabla}[1]{\rowfont[c]{\bfseries}#1\midrule}
\newcommand{\celdadoble}[2]{\begin{tabular}{@{}#1@{}}#2\end{tabular}}
\newcolumntype{L}[1]{>{\raggedright\let\newline\\\arraybackslash\hspace{0pt}}m{#1}}
\newcolumntype{C}[1]{>{\centering\let\newline\\\arraybackslash\hspace{0pt}}m{#1}}
\newcolumntype{R}[1]{>{\raggedleft\let\newline\\\arraybackslash\hspace{0pt}}m{#1}}


% Configuracion de los titulos
\renewcommand{\thesection}{\arabic{section}}
\renewcommand{\thesubsection}{\arabic{section}.\arabic{subsection}}
\definecolor{color_cabecera1}{RGB}{13,51,96}
\definecolor{color_barra1}{RGB}{65,126,195}
\definecolor{color_cabecera2}{RGB}{42,92,150}
\newcommand{\seccion}[1]{\pagebreak\section{#1}}
\newcommand{\subseccion}[1]{\subsection{#1}}
%\newcommand{\subapartado}[1]{\subsubsection{#1}}
%\newcommand{\parrafo}[1]{\paragraph{#1}}
%\newcommand{\pregunta}[1]{\paragraph{#1}}

\setcounter{tocdepth}{2}

\titleformat{\section}
{\normalfont\Large\bfseries\color{color_cabecera1}}
{\thesection.}{1em}{}[{\color{color_barra1}\titlerule[0.8pt]}]

\titleformat{\subsection}
{\normalfont\large\bfseries\color{color_cabecera2}}
{\thesubsection.}{1em}{}[]

\titleformat{\subsubsection}
{\normalfont\bfseries}
{\thesubsection.\thesubsubsection.}{0.4em}{}[]

% Headers & footers
\RequirePackage{fancyhdr}
\pagestyle{fancy}
\fancyhf{}

\setlength\headheight{33pt}
\lhead{\includegraphics[height=1cm]{img/UAM.JPG}}
\chead{\shortstack[r]{
\normalfont\Large\bfseries \myasignatura	\\
3º del Grado en Ingeniería Informática  	%%
}}
\rhead{\shortstack[l]{
\color{color_barra1}\bfseries Curso \mycurso         	\\
\color{color_barra1}\bfseries Práctica \mynumpractica	%%
}}
\lfoot{\qquad\thepage}
\rfoot{\myautores}
\renewcommand{\headrulewidth}{1pt}
\renewcommand{\footrulewidth}{1pt}

% Listados del codigo
\renewcommand{\lstlistingname}{Listado}
\definecolor{col_keyword}{RGB}{200,68,68}	% 68,85,136
\definecolor{col_comment}{RGB}{0,63,139} 	% 128,128,128
\definecolor{col_string}{RGB}{90,150,40} 	% 100,85,85
\lstset{
	language=Lisp, % [ANSI]C
	basicstyle=\ttfamily\footnotesize,
	breaklines=true,
	postbreak=\raisebox{0ex}[0ex][0ex]{\ensuremath{\color{red}\hookrightarrow\space}},
	captionpos=b,
	frame=leftline,
	keepspaces=true,
	numbers=left,
	tabsize=4,
	title=\lstname,
	keywordstyle=\color{col_keyword}\ttfamily\bfseries,
	morekeywords={typedef,struct,unsigned,int,pthread_t},
	stringstyle=\color{col_string}\ttfamily,
	commentstyle=\color{col_comment}\ttfamily,
	morecomment=[is]{/**}{*/}, % Excluye JavaDoc
	literate= % Permite castellano correcto en el codigo
	{á}{{\'a}}1 {é}{{\'e}}1 {í}{{\'i}}1 {ó}{{\'o}}1 {ú}{{\'u}}1
	{Á}{{\'A}}1 {É}{{\'E}}1 {Í}{{\'I}}1 {Ó}{{\'O}}1 {Ú}{{\'U}}1
	{ü}{{\"u}}1 {Ü}{{\"U}}1 {ñ}{{\~n}}1 {Ñ}{{\~N}}1 {€}{{\EUR}}1
	{¬}{{$\neg$}}1
}
\definecolor{terminal_backgroundcolor}{RGB}{250,250,255}
\lstdefinestyle{terminal}{
	backgroundcolor=\color{terminal_backgroundcolor},
	numbers=none,
	frame=single, % top
	keepspaces=true,
}

\definecolor{color_codigo}{RGB}{40,40,40}
\definecolor{color_fichero}{RGB}{30,20,70}
\definecolor{color_funcion}{RGB}{0,100,0}
\newcommand{\codigo}[1]{\texttt{\color{color_codigo}#1}}
\newcommand{\fichero}[1]{\texttt{\color{color_fichero}#1}}
\newcommand{\funcion}[1]{\texttt{\textcolor{color_funcion}{#1}()}}

\newcommand{\numero}[2][5]{\num[round-mode=places,round-precision=#1]{#2}}

% Bytes
\newcommand{    \bytes}[1]{\SI{#1}{\byte}}
\newcommand{\kilobytes}[1]{\SI{#1}{\kilo\byte}}
\newcommand{\megabytes}[1]{\SI{#1}{\mega\byte}}
% Bits
\newcommand{    \bits}[1]{\SI{#1}{\bit\per\second}}
\newcommand{\kilobits}[1]{\SI{#1}{\kilo\bit\per\second}}
\newcommand{\megabits}[1]{\SI{#1}{\mega\bit\per\second}}

% Negrita y cursiva
\newcommand{\negrita}[1]{\textbf{#1}}
\newcommand{\cursiva}[1]{\textit{#1}}
\newcommand{\subrayado}[1]{\underline{#1}}
% Semantico
\newcommand{\importante}[1]{\negrita{#1}}
\newcommand{\anotacion}[1]{\cursiva{#1}}

% Portada
\newcommand{\HRule}{\rule{\linewidth}{0.5mm}}
\newcommand{\portada}[1]{
	\pagenumbering{roman}
	\begin{titlepage}
	\begin{center}

		\HRule                                             	\\[1em]
		{ \LARGE\bfseries \mypractica }                    	\\
		\HRule                                             	\\[4em]
		{ \Large \textsc{\myuniversidad -- \myasignatura} }	\\[3em]
		{ \Large \myautores }                              	\\[0.5em]
		{ \today }                                         	\\[0.5em]
		#1

	\end{center}
	\end{titlepage}
	\thispagestyle{empty}
}

% Tabla de contenidos
\newcommand{\indice}{
	\newpage\tableofcontents
	\newpage\pagenumbering{arabic}
	\stepcounter{page}\stepcounter{page}
}

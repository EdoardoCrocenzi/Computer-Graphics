# Yocto/Modeling: Tiny Procedural Modeler

In questo homework, impareremo i principi di base del procedural modeling.
In particolare, impareremo come

- generare terreni,
- generare capelli,
- generare instanze

## Framework

Il codice utilizza la libreria [Yocto/GL](https://github.com/xelatihy/yocto-gl),
che è inclusa in questo progetto nella directory `yocto`.
Si consiglia di consultare la documentazione della libreria che si trova
all'inizio dei file headers. Inoltre, dato che la libreria verrà migliorata
durante il corso, consigliamo di mettere star e watch su github in modo che
arrivino le notifiche di updates.

Il codice è compilabile attraverso [Xcode](https://apps.apple.com/it/app/xcode/id497799835?mt=12)
su OsX e [Visual Studio 2019](https://visualstudio.microsoft.com/it/vs/) su Windows,
con i tools [cmake](www.cmake.org) e [ninja](https://ninja-build.org)
come mostrato in classe e riassunto, per OsX,
nello script due scripts `scripts/build.sh`.
Per compilare il codice è necessario installare Xcode su OsX e
Visual Studio 2019 per Windows, ed anche i tools cmake e ninja.
Come discusso in classe, si consiglia l'utilizzo di
[Visual Studio Code](https://code.visualstudio.com), con i plugins
[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) e
[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
extensions, che è già predisposto per lavorare su questo progetto.

Questo homework consiste nello sviluppare varie procedure che nella libreria 
`yocto_model`, incluse nel file `yocto_model.cpp`. Le procedure che scriverete
sono eseguite da `ymodel.cpp` come interfaccia a riga di comando.
Includiamo anche i renderer di Yocto/GL in `yscene.cpp` usati per le 
immagini finali.

Questo repository contiene anche tests che sono eseguibili da riga di comando
come mostrato in `run.sh`. Le immagini generate dal runner sono depositate
nella directory `out/`, mentre le scene sono depositate in `outs/`.
Questi risultati devono combaciare con le immagini nella
directory `check/`.

## Funzionalità (24 punti)

In questo homework verranno implementate le seguenti funzionalità:

- **Procedural Terrain** nella funziont `make_terrain()`:
  - creare il terraon spostando i vertici lungo la normale
  - l'altezza di ogni vertice va calcolata con un `ridge()` noise,
    che dovete implementare, moltiplicato per `1 - (pos - center) / size`
  - applicare poi i colori ai vertici in base all'altezza, `bottom`
    per il 33%, `middle` dal 33% al 66% e `top` il resto
  - alla fine calcolate vertex normals usando le funzioni in Yocto/Shape
- **Procedural Dispalcement** nella funzione `make_displacement()`:
  - spostare i vertici lungo la normale per usando `turbulence()` noise,
    che dovete implementare
  - colorare ogni vertice in base all'altezza tra `bottom` e `top`
  - alla fine calcolate vertex normals usando le funzioni in Yocto/Shape
- **Procedural Hair** nella funzione `make_hair()`:
  - generare `num` capelli a partire dalla superficie in input
  - ogni capello è una linea di `steps` segmenti
  - ogni capello inizia da una punto della superficie ed e' direzionato
    lungo la normale
  - il vertice successivo va spostato lungo la direzione del capello di
    `length/steps`, perturbato con `noise()` e spostato lungo l'asse y
    di una quantità pari alla gavità
  - il colore del capelli varia lungo i capelli da `bottom` a `top`
  - per semplicità abbiamo implementato la funzione `sample_shape()` per
    generare punti sulla superficie e `add_polyline()` per aggiungere un
    "capello" ad una shape
  - alla fine calcolate vertex tangents usando le funzioni in Yocto/Shape
- **Procedural Grass** nella funzione `make_grass()`:
  - generare `num` fili d'erba instanziando i modelli dati in input
  - per ogni filo d'erba, scegliere a random l'oggetto da instanziare e
    metterlo sulla superficie in un punto samplato e orientato lungo la normale,
    creando una istanza
  - per dare variazione transformare l'instanza applicando, in questo ordine,
    uno scaling random tra 0.9 e 1.0, una rotazione random attorno all'asse
    z tra 0.1 e 0.2, e una rotazione random attorno all'asse y tra 0 e 2 pi.

## Extra Credit (10 punti)

Anche in questo homework suggeriamo extra credit. Scegliete gli extra credit
che desiderate. Il punteggio sarà la somma dei punteggi degli extra credits
corrtti, fino al massimo indicato sopra.

- **Density Control** (facile):
  - aggoiungere un modo per controllare la generazione di elementi in base
    ad una mappa di densità per controllare gli esempi di capelli o erba
  - per ogni elemento che si vuole aggiungere fare un check se un numero
    casule è minorr della densità
- **Better Procedurals** (facile):
  - scrivere una nuova procedure che migliora i risultati ottenuti dai
    generatori precedenti, come ad esempio
    - displacement modellati su superfici reali
    - capelli molto lunghi con forme interessanti, tipo flow noise
    - esempi da ShaderToy
- **Sample Elimination** (medio):
  - migliorare il posizionamento di punti sulla superficie;
    mostrare i risulati mettendo punti o piccole sfere sulla superficie e
    confrontando random sampling e questi metodi
  - seguire la pubblicazione [sample elimination](http://www.cemyuksel.com/research/sampleelimination/)
  - l'algoritmo richiede di trovare dei nearest neighbors; per farlo usate
    o `hash_grid` in Yocto/Shape o `nanoflann` su GitHub
- **Poisson Point Set**, (medio)
  - seguire la pubbliacazione in
    [poisson sampling](https://www.cct.lsu.edu/~fharhad/ganbatte/siggraph2007/CD2/content/sketches/0250.pdf)
  - trovate varie implementazioni di questo metodo, inclusa una dell'autore
  - dimostrate il risultato mettendo punti su un piano
- **Procedural Noises** (facile):
  - implementate altri patterns di tipo procedurale prendendo spunto da ShaderToy
  - esempi possibile [Voronoise](https://www.iquilezles.org/www/articles/voronoise/voronoise.htm),
    [PhasorNoise](https://www.shadertoy.com/view/WtjfzW),
    [CellNoise](https://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm),
    [SmoothVoronoi](https://www.iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm)
  - dimostrate il risultato su una superficie
- **Alberi**, (difficile):
  - implementare un generatore di alberi che segue l'algoritmo presentato
    [qui](http://algorithmicbotany.org/papers/colonization.egwnp2007.large.pdf)
  - [esempio con animazioni](https://ciphrd.com/2019/09/11/generating-a-3d-growing-tree-using-a-space-colonization-algorithm/)
  - [esempio di risultati artistici](https://nccastaff.bournemouth.ac.uk/jmacey/MastersProject/MSc17/01/report.pdf)
  - per ogni segnmento potete usare un cilindro o una linea
  - il risultato va bene anche senza generare le foglie, l'importante è la forma degli alberi

Per la consegna, il contenuto dell'extra credit va descritto in un PDF chiamato
**readme.pdf**, in aggiunta alla consegna di codice e immagini.
Basta mettere per ogni extra credit, il nome dell'extra credit,
un paragrafo che descriva cosa avete implementato, una o più immagini dei
risulato, e links a risorse utilizzate nell'implementazione.

Potete produrre il PDF con qualsiasi strumento. Una possibilità comoda è di
scrivere il file in Markdown e convertilo in PDF con VSCode plugins o altri
strumenti che usate online. In questo modo potete linkare direttamente alla
immagini che consegnate.

## Istruzioni

Per consegnare l'homework è necessario inviare una ZIP che include il codice e
le immagini generate, cioè uno zip _con le sole directories `yocto_model` e `out`_.
Per chi fa l'extra credit, includere anche `apps`, ulteriori immagini, e un
file **`readme.pdf`** che descrive la lista di extra credit implementati.
Per chi crea nuovi modelli, includere anche le directory con i nuovo modelli creati.
Il file va chiamato `<cognome>_<nome>_<numero_di_matricola>.zip`
e vanno escluse tutte le altre directory. Inviare il file su Google Classroom.

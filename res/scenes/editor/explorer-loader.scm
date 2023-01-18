
(define (createScenegraph name title topic texturepath basenumber values)
  (mk-obj-attr name
    (list
      (list "script" "./res/scenes/editor/scenegraph.scm")
      (list "depgraph" "raw")
      (list "gentexture" texturepath)
      (list "basenumber" basenumber)
      (list "title" title)
      (list "topic" topic)
      (list "values" (string-join values "|"))
    )
  )
  (format #t "create scenegraph placeholder\n")
)

(define explorerInstance #f)

(define (unloadExplorer)
  (if explorerInstance
    (unload-scene explorerInstance)
  )
  (set! explorerInstance #f)
)

(define messageToExplorer
  (list
    (list "load-test" "Test Menu" "./res/textures/wood.jpg")
    (list "load-sound" "Sound Values" "explorer-gentexture-sound")
    (list "load-heightmap" "Heightmap Values" "explorer-gentexture-heightmap")
    (list "load-heightmap-brush" "Heightmap Brushes" "explorer-gentexture-heightmap-brush")
  )
)
(define (loadExplorer key)
  (define value (assoc key messageToExplorer))
  (define title (cadr value))
  (define texture (caddr value))
  (define sceneId 
    (load-scene 
      "./res/scenes/editor/explore.rawscene"
      (list
        (list "(dialog" "tint" "0 1 1 0.2")
        (list ")text_main" "value" title)
        (list "*basicbutton1" "ontexture" texture)
      )
    )
  )
  (format #t "load explorer: ~a\n" value)
  (set! explorerInstance sceneId)
)

(define (handleLoad value)
  (unloadExplorer)
  (loadExplorer value)
)


(define explorerSoundValue #f)
(define explorerHeightmapValue #f)
(define explorerHeightmapBrushValue #f)
(define (onMessage key value)
  (format #t "explorer loader: ~a ~a\n" key value)
  (if (equal? key "explorer-sound")
    (set! explorerSoundValue value)
  )
  (if (equal? key "explorer-heightmap-brush")
    (set! explorerHeightmapBrushValue value)
  )
  (if (equal? key "explorer-heightmap")
    (set! explorerHeightmapValue value)
  )
  (if (equal? key "explorer")
    (cond 
      ((equal? value "explorer-ok")
        (begin
          (if explorerSoundValue
            (send "explorer-sound-final" explorerSoundValue)
          )
          (set! explorerSoundValue #f)
 
          (if explorerHeightmapBrushValue
            (send "explorer-heightmap-brush-final" explorerHeightmapBrushValue)
          )
          (set! explorerHeightmapBrushValue #f)        

          (if explorerHeightmapValue
            (send "explorer-heightmap-final" explorerHeightmapValue)
          )
          (set! explorerHeightmapValue #f)
          (unloadExplorer)
        )
      )
      ((equal? value "explorer-cancel")
        (unloadExplorer)
      )
      ((or (equal? value "load-sound") (equal? value "load-test") (equal? value "load-heightmap") (equal? value "load-heightmap-brush"))
        (handleLoad value)
      )
    )
  )
)


(createScenegraph 
  "|fileexplorer-sound"
  "Sound List"
  "explorer-sound"
  "explorer-gentexture-sound"
  30000
  (ls-res "sounds")
)

(createScenegraph
  "|fileexplorer-heightmap"
  "Heightmap List"
  "explorer-heightmap"
  "explorer-gentexture-heightmap"
  40000
  (append (ls-res "heightmaps"))
)

(createScenegraph
  "|fileexplorer-heightmap-brush"
  "Heightmap Brushes"
  "explorer-heightmap-brush"
  "explorer-gentexture-heightmap-brush"
  50000
  (append (ls-res "heightmap-brushes"))
)

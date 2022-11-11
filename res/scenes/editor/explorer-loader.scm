
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
    (list "load-sound" "Sound Values" "gentexture-raw")
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

(define (onMessage key value)
  (format #t "explorer loader: ~a ~a\n" key value)
  (if (equal? key "explorer")
    (cond 
      ((equal? value "explorer-ok")
        (format #t "explorer ok placeholder\n")
      )
      ((equal? value "explorer-cancel")
        (unloadExplorer)
      )
      ((or (equal? value "load-sound") (equal? value "load-test"))
        (handleLoad value)
      )
    )
  )
)


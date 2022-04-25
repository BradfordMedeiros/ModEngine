(define uiOption "file")
(define uilist 
  (list
    (list "file" (list "open" "save" "load" "quit"))
    (list "misc" (list "fullscreen"))
  )
)

(define (textvalue name content)
  (list name
    (list 
      (list name "layer" "basicui")
      (list name "scale" "0.005 0.015 0.005")
      (list name "value" content)
    )
  )
)
(define (popover-options menuOptions)
  (define index -1)
  (define (nextName)
    (format #t "next value\n")
    (set! index (+ index 1))
    (string-append ")text_" (number->string index))
  )
  (define (create-value text) (textvalue (nextName) text))

  (define val (map create-value menuOptions))
  (define itemnames (map car val))
  (define elements (map cadr val))
  (define joinedNames (string-join itemnames ","))
  (define joinedElements (apply append elements))
  (define baselist (list (list "(dialog" "elements" joinedNames)))
  (format #t "joinedNames: ~a\n" joinedNames)
  (format #t "joinedElements name: ~a\n" joinedElements)
  (append baselist  joinedElements)
)

(load-scene "./res/scenes/editor/popover.rawscene" (popover-options (cadr (assoc uiOption uilist))))

;(define (onObjSelected gameobj color)
;  (format #t "on object selected: ~a ~a" gameobj color)
;  (format #t "name: ~a\n" (gameobj-name gameobj))
;)
(define uilist 
  (list
    (list "file" (list "open" "save" "load" "quit"))
    (list "misc" (list "fullscreen"))
  )
)
(define menuOptions (map car uilist))

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

(define sceneId #f)
(define (change-popover uiOption)
  (if sceneId
    (unload-scene sceneId)
  )
  (set! sceneId (load-scene "./res/scenes/editor/popover.rawscene" (popover-options (cadr (assoc uiOption uilist)))))
  (format #t "object id: ~a\n" (lsobj-name "(dialog" sceneId))
  (enforce-layout (gameobj-id (lsobj-name "(dialog" sceneId)))
)


(define (onObjSelected gameobj color)
  (define popoptionPair (assoc "popoption" (gameobj-attr gameobj)))
  (define popoption (if popoptionPair (cadr popoptionPair) ""))
  (define isInList (not (equal? #f (member popoption menuOptions))))
  (format #t "popoption: ~a\n" popoption)
  (if isInList
    (change-popover popoption)
  )
)



; Make the selection type populate the popover correctly 



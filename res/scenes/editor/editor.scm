(define mainSceneId (list-sceneid (gameobj-id mainobj)))

(define uilist 
  (list
    (list "file" (reverse (list "open" "save" "load" "quit")))
    (list "misc" (reverse (list "fullscreen")))
  )
)

(define nameAction (list
  (list "open" (lambda () (format #t "open placeholder\n")) )
  (list "save" (lambda () ( format #t "save placeholder\n")) )
  (list "load" (lambda () ( format #t "load placeholder\n")) )
  (list "quit" (lambda () (exit 0)) )
  (list "fullscreen" (lambda () 
      (format #t "toggling fullscreen\n")
      (set-wstate (list
        (list "rendering" "fullscreen" "true") ; doesn't actually toggle since fullscreen state never updates to match internal with set-wstate
      ))
    )
  )
))
 
(define menuOptions (map car uilist))

(define (textvalue name content action)
  (list name
    (list 
      (list name "layer" "basicui")
      (list name "scale" "0.005 0.015 0.005")
      (list name "value" content)
      (list name "popaction" action)
    )
  )
)
(define (popoverAction action)
  (define mappedAction (assoc action nameAction))
  (if mappedAction ((cadr mappedAction)))
)

[define popItemPrefix ")text_"]
(define popItemPrefixLength (string-length popItemPrefix))
(define (popover-options elementName menuOptions)
  (define index -1)
  (define (nextName)
    (format #t "next value\n")
    (set! index (+ index 1))
    (string-append popItemPrefix (number->string index))
  )
  (define (create-value text) (textvalue (nextName) text text))

  (define val (map create-value menuOptions))
  (define itemnames (map car val))
  (define elements (map cadr val))
  (define joinedNames (string-join itemnames ","))
  (define joinedElements (apply append elements))
  (define baselist (list 
    (list "(dialog" "elements" joinedNames)
    (list "(dialog" "anchor" elementName)
  ))
  (format #t "joinedNames: ~a\n" joinedNames)
  (format #t "joinedElements name: ~a\n" joinedElements)
  (format #t "element name is: ~a\n" elementName)
  (append baselist  joinedElements)
)

(define sceneId #f)
(define (maybe-unload-popover) 
  (if sceneId (unload-scene sceneId))
  (set! sceneId #f)
)
(define (change-popover elementName uiOption)
  (maybe-unload-popover)
  (set! sceneId (load-scene "./res/scenes/editor/popover.rawscene" (popover-options elementName (cadr (assoc uiOption uilist)))))
  (format #t "object id: ~a\n" (lsobj-name "(dialog" sceneId))
  (enforce-layout (gameobj-id (lsobj-name "(dialog" sceneId)))
  (enforce-layout (gameobj-id (lsobj-name "(dialog" sceneId)))

)


(define (isPopoverElement name) (if (< (string-length name) popItemPrefixLength) #f (equal? (substring name 0 popItemPrefixLength) ")text_")) )

(define (fullElementName localname) (string-append "." (number->string mainSceneId) "/" localname))
(define (onObjSelected gameobj color)
  (define objattrs (gameobj-attr gameobj))
  (define popoptionPair (assoc "popoption" objattrs))
  (define popactionPair (assoc "popaction" objattrs))
  (define popoption (if popoptionPair (cadr popoptionPair) ""))
  (define isInList (not (equal? #f (member popoption menuOptions))))
  (define elementName (gameobj-name gameobj))
  (define ispopover (isPopoverElement elementName))
  (format #t "popoption: ~a\n" popoption)
  (if isInList
    (change-popover (fullElementName elementName) popoption)
  )
  (if (and ispopover popactionPair) 
    (popoverAction (cadr popactionPair))
  )
  (if (not (or isInList ispopover)) (maybe-unload-popover))
)



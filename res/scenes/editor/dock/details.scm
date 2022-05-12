(define mainSceneId (list-sceneid (gameobj-id mainobj)))

(define (isManagedText gameobj)
  (and 
    (equal? (list-sceneid (gameobj-id gameobj)))
    (equal? ")" (substring (gameobj-name gameobj) 0 1))
  )
)


(define (updateText obj text)
  (gameobj-setattr! obj 
    (list
      (list "value" text)
    )
  )
)

(define (lessIndex currentText) (max 0 (- (string-length currentText) 1)))
(define (getUpdatedText attr obj key)
  (define currentText (cadr (assoc "value" attr)))   
  (if (= key 259)  ; backspace
    (set! currentText (substring currentText 0 (lessIndex currentText)))
    (if (= key 257)
      (set! currentText (string-append currentText "\n"))
      (begin
        (format #t "key is ~a ~a\n" key (string (integer->char key)))
        (set! currentText (string-append currentText (string (integer->char key))))
      )
    )
  )
  currentText
)

(define (shouldUpdateText attr)
  (define editableText (assoc "details-editabletext" attr))
  (and editableText (equal? "true" (cadr editableText)))
)

(define focusedElement #f)
(define (processFocusedElement key)
  (if focusedElement
    (begin
      (format #t "process focus elmement placeholder: ~a\n" key)
      (let ((attr (gameobj-attr focusedElement)))
        (if (shouldUpdateText attr) 
          (updateText focusedElement (getUpdatedText (gameobj-attr focusedElement) focusedElement key))
        )
      )
    )
  )
)

(define (onObjSelected gameobj color)
  (if (equal? (gameobj-id gameobj) (gameobj-id mainobj))
    (sendnotify "dock-self-remove" (number->string (gameobj-id mainobj)))
  )

  (if (isManagedText gameobj)
    (begin
      (format #t "is is a managed element: ~a\n" (gameobj-name gameobj))
      (set! focusedElement gameobj)
    )
    (set! focusedElement #f)
  )

)

;; todo remove - no items in this layout, should require this 
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))


(define (onKey key scancode action mods)
  (if (equal? action 1)
    (processFocusedElement key)
  )
)
(define mainSceneId (list-sceneid (gameobj-id mainobj)))
(define mainpanelId (gameobj-id (lsobj-name "(test_panel")))

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
  (enforce-layout mainpanelId)
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


(define (object-should-be-active gameobj id)   ; should use details-group-index instead
  (list gameobj (equal? id (gameobj-id gameobj)))
)
(define (set-object-active-state gameobjActivePair)
  (gameobj-setattr! (car gameobjActivePair) (list 
    (list "tint" (if (cadr gameobjActivePair) (list 0 0 1 1) (list 1 1 1 1)))
  ))
)
(define (handleListSelection gameobj detailsAttr)
  (if detailsAttr
    (let ((groupObjs (lsobj-attr "details-group" (cadr detailsAttr))))
      (for-each set-object-active-state (map (lambda(obj) (object-should-be-active obj (gameobj-id gameobj))) groupObjs))
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

  (handleListSelection gameobj (assoc "details-group"  (gameobj-attr gameobj)))
)

;; todo remove - no items in this layout, should require this 
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))


(define (onKey key scancode action mods)
  (if (equal? action 1)
    (processFocusedElement key)
  )

  (format #t "elements with attr: marked ~a\n" (lsobj-attr "marked"))
  (format #t "element names: ~a\n" (map gameobj-name (lsobj-attr "marked")))
)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (create-attr-pair gameobj) (list gameobj (gameobj-attr gameobj)))
(define (get-binded-elements) (map create-attr-pair (lsobj-attr "details-binding")))
(define (extract-binding-element attr-pair) (list (car attr-pair) (cadr (assoc "details-binding" (cadr attr-pair)))))
(define (all-obj-to-bindings) (map extract-binding-element (get-binded-elements)))
(define (generateGetDataForAttr attributeData)
  (lambda(attrField) 
    (let ((fieldPair (assoc attrField attributeData)))
      (if fieldPair (cadr fieldPair) "< no data source >") 
    )
  )
)
(define (update-binding attrpair getDataForAttr) 
  (gameobj-setattr! (car attrpair) 
    (list (list "value" (getDataForAttr (cadr attrpair))))
  )
  attrpair
)

(define (populateData)
  (define dataValues   
    (list
      (list "object_name" "test object")
      (list "lighttype" "bright")
      (list "position" "- 0 0 0")
    )
  )
  (define getDataForAttr (generateGetDataForAttr dataValues))
  (map (lambda(attrpair) (update-binding attrpair getDataForAttr)) (all-obj-to-bindings))
)

(populateData)

;;;;;;;;;;;;;;;

(define (onKeyChar key)
  ; ".
  (if (equal? key 46)
    (format #t "Submit bindings placeholder\n")
  ) 
)

(define (createCameraPlaceholder) (format #t "placeholder to create camera!\n"))
(define (createLightPlaceholder) (format #t "placeholder to create light!\n"))
(define buttonToAction
  (list
    (list "create-camera" createCameraPlaceholder)
    (list "create-light" createLightPlaceholder)
  )
)
(define (perform-button-action obj)
  (define attrActions (assoc "button-action" (gameobj-attr obj)))
  (format #t "attr actions: ~a\n " attrActions)
  (if attrActions
    (let ((action (assoc (cadr attrActions) buttonToAction)))
      (if action
        ((cadr action))
      )
    )
  )
)
(define (onMessage key value)
  (if (equal? key "dialog-button-action") 
    (perform-button-action (gameobj-by-id (string->number value)))
  )
)
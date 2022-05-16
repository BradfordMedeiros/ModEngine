; 1. details-binding:<attribute value>, this makes the value property get populated with the value in this
; 2. details-group:<attribute value>, details-index: <attributevalue>,  
; ==> if element is selected with same group, the corresponding tint for all other elements is set low if detail-index is different, else high
; 3. if dialog-button-action message is sent, will perform the action in value of attribute button-action for the corresponding object id in the message
; 4. manages value field (as text editor style functionality) for focused elements with details-editabletext:true


(define (onKeyChar key)
  ; ".
  (if (equal? key 46)
    (format #t "data values: ~a\n" dataValues)
  ) 
)

(define dataValues (list
))

(define (clearStore) (set! dataValues (list)))
(define (updateStoreValue keyvalue)
  (define key (car keyvalue))
  (define value (cadr keyvalue))
  (define newDataValues (delete (assoc key dataValues) dataValues))
  (set! dataValues (cons (list key value) newDataValues))
  (format #t "store:  data values is: ~a\n" dataValues)
)

(define (createCameraPlaceholder) (format #t "placeholder to create camera!\n"))
(define (createLightPlaceholder) (format #t "placeholder to create light!\n"))
(define buttonToAction
  (list
    (list "create-camera" createCameraPlaceholder)
    (list "create-light" createLightPlaceholder)
  )
)


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
      (let ((attr (gameobj-attr focusedElement)))
        (if (shouldUpdateText attr) 
          (updateText focusedElement (getUpdatedText (gameobj-attr focusedElement) focusedElement key))
        )
      )
    )
  )
)


(define (object-should-be-active gameobj selectedGroupIndex)   ; should use details-group and details-group-index instead
  (define groupIndex (assoc "details-group-index" (gameobj-attr gameobj)))
  (if groupIndex
    (if (equal? (cadr groupIndex) selectedGroupIndex) (list gameobj #t) (list gameobj #f))
    (list gameobj #f)
  )
)
(define (set-object-active-state gameobjActivePair)
  (gameobj-setattr! (car gameobjActivePair) (list 
    (list "tint" (if (cadr gameobjActivePair) (list 0 0 1 1) (list 1 1 1 1)))
  ))
)
(define (update-group-values group selectedIndex)
  (define groupObjs (lsobj-attr "details-group" group))
  (for-each set-object-active-state (map (lambda(obj) (object-should-be-active obj selectedIndex)) groupObjs))
)

(define (handleListSelection gameobj selectedAttr)
  (define detailsAttr (assoc "details-group" selectedAttr))
  (define selectedGroupIndex (assoc "details-group-index" selectedAttr))
  (if (and detailsAttr selectedGroupIndex) 
    (let ((group (cadr detailsAttr)) (selectedIndex (cadr selectedGroupIndex)))
      (updateStoreValue (list group selectedIndex))
      (format #t "updated group: ~a with index ~a\n" group selectedIndex)
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

  (clearStore)
  (updateStoreValue (list "object_name" (gameobj-name gameobj)))
  (format #t "store: all attrs are: ~a\n" (gameobj-attr gameobj))
  (map updateStoreValue (gameobj-attr gameobj))
  (handleListSelection gameobj (gameobj-attr gameobj))
  (populateData)
)

;; todo remove - no items in this layout, should require this 
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))


(define (onKey key scancode action mods)
  (if (equal? action 1)
    (processFocusedElement key)
  )
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

(define (extract-group-element attr-pair) 
  (define attrPair (cadr attr-pair))
  (list 
    (car attr-pair) 
    (cadr (assoc "details-group" attrPair))
  )
)
(define (get-group-elements) (map create-attr-pair (lsobj-attr "details-group")))
(define (all-obj-to-group-bindings) (map extract-group-element (get-group-elements)))
(define (get-details-group allobjs)
  (define uniqueGroups (list))

  (format #t "all objs: ~a\n" allobjs)
  (for-each (lambda(objPair) 
    (let ((detailsGroup (cadr objPair)))
      (if (not (member detailsGroup uniqueGroups))
        (set! uniqueGroups (cons detailsGroup uniqueGroups))
      )
    )
  ) allobjs)
  uniqueGroups
)


(define (get-group-value group)
  (define groupValue (assoc group dataValues))
  (if groupValue
    (list group (cadr groupValue))
    (list group #f)
  )
)
(define (get-group-values )
  (define uniqueGroups (get-details-group (all-obj-to-group-bindings)))
  (map get-group-value uniqueGroups)
)

(define (populateData)
  (define getDataForAttr (generateGetDataForAttr dataValues))
  (for-each 
    (lambda(attrpair) 
      (update-binding attrpair getDataForAttr)
    ) 
    (all-obj-to-bindings)
  )
  (for-each 
    (lambda(groupValuePair) 
      (format #t "group value pair: ~a\n" groupValuePair)
      (if (cadr groupValuePair)
        (update-group-values (car groupValuePair) (cadr groupValuePair))
      )
    ) 
    (get-group-values)
  )
)


;;;;;;;;;;;;;;;


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
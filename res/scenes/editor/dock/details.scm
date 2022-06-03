; 1. details-binding:<attribute value>, this makes the value property get populated with the value in this
; 2. details-group:<attribute value>, details-index: <attributevalue>,  
; ==> if element is selected with same group, the corresponding tint for all other elements is set low if detail-index is different, else high
; 3. if dialog-button-action message is sent, will perform the action in value of attribute button-action for the corresponding object id in the message
; 4. manages value field (as text editor style functionality) for focused elements with details-editabletext:true


(define (isSubmitKey key) (equal? key 47))   ; /
(define (isControlKey key) (or (isSubmitKey key) (equal? key 44)))
(define (submitData)
  (if managedObj
    (begin
      (let ((updatedValues (filterUpdatedObjectValues)))
        (format #t "values to update: ~a\n" updatedValues)
        (gameobj-setattr! managedObj updatedValues)
        (format #t "submitted values!\n")
        ;; temporary, just to get the effect, should change
        ;(for-each (lambda(attrPair) 
        ;  (if (equal? "editor-eoe-mode" (car attrPair))
        ;    (begin
        ;      (format #t "new eoe mode: ~a\n" (cadr attrPair))
        ;      (format #t "not eoe -> ~a\n" attrPair)
        ;      (if (equal? (cadr attrPair) "enabled") (set! eoeMode #t))
        ;      (if (equal? (cadr attrPair) "disabled") (set! eoeMode #f))
        ;    )
        ;  )
        ;) updatedValues)
      )
    )
  )
)
(define (onKeyChar key)
  (format #t "key is: ~a\n" key)
  (if (equal? key 44) ; comma
    (format #t "~a\n" dataValues) 
  )
  (if (isSubmitKey key) (submitData))
)


(define dataValues (list))

(define (isUpdatedObjectValue dataValue) (equal? #t (caddr dataValue)))
(define (getAttr dataValue) (list (car dataValue) (cadr dataValue)))
(define (filterUpdatedObjectValues) (map getAttr (filter isUpdatedObjectValue dataValues)))

(define (clearStore) (set! dataValues (list)))
(define (updateStoreValueModified keyvalue modified)
  (define key (car keyvalue))
  (define value (cadr keyvalue))  ; maybe guess the type here?
  (define newDataValues (delete (assoc key dataValues) dataValues))
  (set! dataValues (cons (list key value modified) newDataValues))
)
(define (updateStoreValue keyvalue) (updateStoreValueModified keyvalue #f))

(define (refillStore gameobj)
  (clearStore)
  (updateStoreValue (list "object_name" (gameobj-name gameobj)))
  (format #t "store: all attrs are: ~a\n" (gameobj-attr gameobj))
  (map updateStoreValue (gameobj-attr gameobj))

  (updateStoreValue (list "editor-eoe-mode" (if eoeMode "enabled" "disabled")))
  (updateStoreValue (list "meta-numscenes" (number->string (length (list-scenes)))))
  (updateStoreValue (list "runtime-id" (number->string (gameobj-id gameobj))))
  (updateStoreValue (list "runtime-sceneid" (number->string (list-sceneid (gameobj-id gameobj)))))
)

(define (createCameraPlaceholder) (format #t "placeholder to create camera!\n"))
(define (createLightPlaceholder) (format #t "placeholder to create light!\n"))
(define (setManipulatorMode mode) (set-wstate (list (list "tools" "manipulator-mode" mode) )))

(define buttonToAction
  (list
    (list "create-camera" createCameraPlaceholder)
    (list "create-light" createLightPlaceholder)
    (list "set-transform-mode" (lambda() (setManipulatorMode "translate")))
    (list "set-scale-mode" (lambda() (setManipulatorMode "scale")))
    (list "set-rotate-mode" (lambda() (setManipulatorMode "rotate")))
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

(define (getDataValue attrField) 
  (define value (assoc attrField dataValues))
  (if value (cadr value) #f)
)

(define (makeTypeCorrect oldvalue newvalue)
  (if (number? oldvalue)
    (if (string? newvalue) 
      (if (equal? (string-length newvalue) 0) 0 (string->number newvalue)) 
      newvalue
    )
    newvalue
  )
)
(define (getUpdatedValue detailBindingName detailBindingIndex newvalue)
  (define oldvalue (getDataValue detailBindingName))
  (if detailBindingIndex
    (begin
      (list-set! oldvalue detailBindingIndex (makeTypeCorrect (list-ref oldvalue detailBindingIndex) newvalue))
      (format #t "oldvalue is now #s only ~a\n" (map number? oldvalue))
      (list detailBindingName oldvalue)
    )
    (list detailBindingName newvalue)
  )
)
(define (updateText obj text)
  (define objattr (gameobj-attr obj))
  (define detailBindingPair (assoc "details-binding" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (gameobj-setattr! obj 
    (list
      (list "value" text)
    )
  )
  (if detailBinding 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex text) #t)
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
(define (isEditableType type attr) 
  (define editableText (assoc "details-editable-type" attr))
  (define isEditableType (if editableText (equal? (cadr editableText) type) #f))
  (format #t "is editable type: ~a ~a\n" type isEditableType)
  isEditableType
)

(define focusedElement #f)
(define (processFocusedElement key)
  (if focusedElement
    (begin
      (let ((attr (gameobj-attr focusedElement)))
        (if (shouldUpdateText attr) 
          (let ((newText (getUpdatedText (gameobj-attr focusedElement) focusedElement key)))
            (if (isEditableType "number" attr)
              (if (or (string->number newText) (equal? 0 (string-length newText))) 
                (updateText focusedElement newText)
              )
              (updateText focusedElement newText)
            )
          )
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


(define eoeMode #f)
(define (isSelectableItem layerAttr)
  (if eoeMode 
    (begin
      (updateStoreValue (list "editor-eoe-mode" "disabled"))
      (set! eoeMode #f)
      #t
    )
    (and (not eoeMode)  (if layerAttr (not (equal? "basicui" (cadr layerAttr))) #t))
  )
)

(define (maybe-perform-action objattr)
  (define attrActions (assoc "details-action" objattr))
  (format #t "attr actions: ~a\n " attrActions)
  (if attrActions
    (let ((action (assoc (cadr attrActions) buttonToAction)))
      (if action
        ((cadr action))
        (format #t "no action for ~a\n" (cadr attrActions))
      )
    )
  )
)

(define managedObj #f)
(define (onObjSelected gameobj color)
  (define objattr (gameobj-attr gameobj))
  (if (equal? (gameobj-id gameobj) (gameobj-id mainobj)) ; assumes script it attached to window x
    (sendnotify "dock-self-remove" (number->string (gameobj-id mainobj)))
  )

  (if (isManagedText gameobj)
    (begin
      (format #t "is is a managed element: ~a\n" (gameobj-name gameobj))
      (set! focusedElement gameobj)
    )
    (set! focusedElement #f)
  )

  (if (isSelectableItem (assoc "layer" objattr))
    (begin
      (refillStore gameobj)
      (set! managedObj gameobj)
      (populateData)
    )
  )
  (handleListSelection gameobj objattr)
  (maybe-perform-action objattr)
)

;; todo remove - no items in this layout, should require this 
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
(enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))


(define (onKey key scancode action mods)
  (if (and (equal? action 1) (not (isControlKey key)))
    (processFocusedElement key)
  )
)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (create-attr-pair gameobj) (list gameobj (gameobj-attr gameobj)))
(define (get-binded-elements bindingType) (map create-attr-pair (lsobj-attr bindingType)))
(define (extract-binding-element attr-pair bindingType) 
  (define attrs (cadr attr-pair))
  (define detailBindingIndex (assoc "details-binding-index" attrs))
  (list 
    (car attr-pair) 
    (cadr (assoc bindingType attrs))
    (if detailBindingIndex (inexact->exact (cadr detailBindingIndex)) #f)
  )
)
(define (all-obj-to-bindings bindingType) (map (lambda(attrPair) (extract-binding-element attrPair bindingType)) (get-binded-elements bindingType)))

(define (generateGetDataForAttr defaultValue)
  (lambda(attrField) 
    (let ((fieldPair (getDataValue attrField)))
      (if fieldPair fieldPair defaultValue) 
    )
  )
)
(define (update-binding attrpair getDataForAttr) 
  (define dataValue (getDataForAttr (cadr attrpair)))
  (define bindingIndex (caddr attrpair))

  (format #t "binding index: ~a ~a\n" bindingIndex (number? bindingIndex))
  (if (and bindingIndex (list? dataValue) (< bindingIndex (length dataValue)))
    (set! dataValue (list-ref dataValue bindingIndex))
  )
  (if (number? dataValue)
    (set! dataValue (number->string dataValue))
  )
  (format #t "data value: ~a\n" dataValue)
  (if (string? dataValue)
    (begin
      (gameobj-setattr! (car attrpair) 
        (list (list "value" dataValue))
      )
    )
    (format #t "warning not a string: ~a ~a ~a\n" attrpair dataValue bindingIndex)
  )
  attrpair
)

(define (update-toggle-binding attrpair getDataForAttr)
  (define toggleEnableText (getDataForAttr (cadr attrpair)))
  (define gameobj (car attrpair))
  (define bindingOn (assoc "details-binding-on" (gameobj-attr gameobj)))
  (define enableValueStr (if bindingOn (cadr bindingOn) "enabled"))
  (define enableValue (equal? enableValueStr toggleEnableText))
  ;(format #t "update toggle binding: ~a with value ~a (~a)\n" attrpair enableValue toggleEnableText)
  (gameobj-setattr! gameobj
    (list (list "state" (if enableValue "on" "off")))
  )
)
(define (toggleButtonBinding objid on)
  (define objattr (gameobj-attr (gameobj-by-id objid)))
  (define detailsBinding (assoc "details-binding-toggle" objattr))
  (define onValue (assoc "details-binding-on" objattr))
  (define offValue (assoc "details-binding-off" objattr))
  (format #t "on value, off value: ~a\n ~a\n" onValue offValue)
  (if (and detailsBinding on onValue)
    (updateStoreValueModified (list (cadr detailsBinding) (cadr onValue)) #t)
  )
  (if (and detailsBinding (not on) offValue)
    (updateStoreValueModified (list (cadr detailsBinding) (cadr offValue)) #t)
  )
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
  (for-each 
    (lambda(attrpair) 
      (update-binding attrpair (generateGetDataForAttr  "< no data source >"))
    ) 
    (all-obj-to-bindings "details-binding")
  )
  (for-each 
    (lambda(attrpair) 
      ;(update-binding attrpair getDataForAttr)
      (update-toggle-binding attrpair (generateGetDataForAttr  #f))
    ) 
    (all-obj-to-bindings "details-binding-toggle")  ;
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

(define (onMessage key value)
  (if (equal? key "editor-button-on")
    (begin
      (toggleButtonBinding (string->number value) #t)
      (submitData) ; remove
    )

  )
  (if (equal? key "editor-button-off")
    (begin
      (toggleButtonBinding (string->number value) #f)
      (submitData) ; remove
    )
  )
)
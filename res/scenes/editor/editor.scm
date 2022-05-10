(define mainSceneId (list-sceneid (gameobj-id mainobj)))

(define uilist 
  (list
    (list "file" (reverse (list "load" "quit")))
    (list "misc" (reverse (list "fullscreen")))
  )
)

(define dialogMap (list
  (list "load" (list 
    "Load Layout" 
    "layouts enable different ui workflows"
    (list 
      (list "CANCEL" (lambda() (maybe-unload-dialog)))
      (list "LOAD"   (lambda() (format #t "load placeholder\n")))
    )
  ))
  (list "quit" (list 
    "Confirm QUIT" 
    "are you sure you want to quit?"
    (list 
      (list "CANCEL" (lambda() (maybe-unload-dialog)))
      (list "QUIT"   (lambda() (exit 1)))
    )
  ))
))
(define (get-from-dialog-map value) (cadr (assoc value dialogMap)))
(define (get-option-from-dialog-map value option) 
  (define dialogMapping (get-from-dialog-map value))
  (assoc option (caddr dialogMapping))
)
(define (get-fn-for-dialog-option value option) (cadr (get-option-from-dialog-map value option)))
(define (get-change-dialog value)
  (let ((quitValues (get-from-dialog-map value)))
    (lambda() (change-dialog (car quitValues) (cadr quitValues) (caddr quitValues)) #t)
  )
)

(define nameAction (list
  (list "load" (get-change-dialog "load"))
  (list "quit" (get-change-dialog "quit"))
  (list "fullscreen" (lambda () 
      (format #t "toggling fullscreen\n")
      (set-wstate (list
        (list "rendering" "fullscreen" "true") ; doesn't actually toggle since fullscreen state never updates to match internal with set-wstate
      ))
      #f
    )
  )
))



;;;;;;;;;;;;
; Sidepanel 
(define sidePanelSceneId #f)
(define (change-sidepanel scene anchorElementName)
  (format #t "change sidepanel: elementname: ~a\n" anchorElementName)
  (maybe-unload-sidepanel)
  (if (not sidePanelSceneId)
    (begin
      (set! sidePanelSceneId 
        (load-scene 
          scene
          (list 
            (list "(test_panel" "script" "./res/scenes/editor/dialogmove.scm")  ; doesn't work with anchored element since both rewrite position
            (list "(test_panel" "dialogmove-restrictx" "true")
            (list "(test_panel" "editor-shouldsnap" "true")
            ;(list "(test_panel" "anchor" anchorElementName)
          )
        )
      )
      (format #t "editor: load scene: ~a\n" scene)
      (format #t "sidepanel id is: ~a\n" sidePanelSceneId)
      (enforce-layout (gameobj-id (lsobj-name "(test_panel" sidePanelSceneId)))
    )
  )
)
(define (maybe-unload-sidepanel)
  (if sidePanelSceneId (unload-scene sidePanelSceneId))
  (set! sidePanelSceneId #f)
)


(define xLocationToSnappingPosition
  (list 
    (list 0.99 (list 1.2 -0.1 0))
    (list 0 (list 0.8 -0.1 0))
    (list -1.1 (list -0.8 -0.1 0))
    (list most-negative-fixnum (list -1.2 -0.1 0))
  )
)
(define (getSnappingValue searchVal vals)
  (define firstValue (car vals))
  (define firstValThreshold (car firstValue))
  (if (> searchVal firstValThreshold)
    firstValue
    (if (> (length vals) 1) (getSnappingValue searchVal (cdr vals)) #f)
  )
)
(define (applySnapping gameobj)
  (define snapXLocation (car (gameobj-pos gameobj)))
  (define snappingPair (getSnappingValue snapXLocation xLocationToSnappingPosition))
  (if snappingPair
    (let ((snappingPos (cadr snappingPair)))
      (gameobj-setpos! gameobj snappingPos)
      (enforce-layout (gameobj-id gameobj))
    )
  )
)

(define (maybe-handle-side-panel-drop id) 
  (define gameobj (gameobj-by-id id))
  (define pos (gameobj-pos gameobj))
  (define snapValue (assoc "editor-shouldsnap" (gameobj-attr gameobj)))
  (define shouldSnap (if snapValue (equal? "true" (cadr snapValue)) #f))
  (if shouldSnap (applySnapping gameobj))
)


(define (onMessage key value)
  (if (equal? key "dialogmove-drag-stop") 
    (maybe-handle-side-panel-drop (string->number value))
  )
)


;;;;;;;;;;;;;


(define (genNextName prefix)
  (define index -1)
  (define (nextName)
    (format #t "next value\n")
    (set! index (+ index 1))
    (string-append prefix (number->string index))
  )
  nextName
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DIALOG
;;;;;;;;;;;;;;;;;;;;;;;;;;
(define (dialog-option name value index)
  (list name
    (list 
      (list name "layer" "basicui")
      (list name "scale" "0.004 0.01 0.004")
      (list name "value" value)
      (list name "option_index" (number->string index))
    )
  )
)

(define (dialog-options options)
  (define nextName (genNextName ")option_"))
  (define index -1)
  (define (val-to-option option) 
    (set! index (+ index 1))
    (dialog-option (nextName) (car option) index)
  )
  (define values (map val-to-option options))
  values
)

(define dialogSceneId #f)
(define activeDialogName #f)
(define (change-dialog title subtitle options)
  (if (not dialogSceneId)
    (let ((dialogOpts (dialog-options options)))
      (set! dialogSceneId 
        (load-scene 
          "./res/scenes/editor/dialog.rawscene" 
          (append 
            (list
              (list "(options" "elements" (string-join (map car dialogOpts) ","))
              (list ")text_2" "value" title)
              (list ")text_main" "value" subtitle)
            ) 
              (apply append (map cadr dialogOpts))
            )
        )
      )
    )
  )
)
(define (maybe-unload-dialog)
  (if dialogSceneId (unload-scene dialogSceneId))
  (set! dialogSceneId #f)
  (set! activeDialogName #f)
)

(define (resolve-option-name name attr)  ; check that it's )option_ and then use the option_index value to find what option # it is 
  (define value (assoc "option_index" attr))
  (define isOptionName (and (>= (string-length name) 8) (equal? (substring name 0 8) ")option_")))
  (if isOptionName
    (let ((intValue (inexact->exact (floor (cadr value)))))
      (car (list-ref (caddr (get-from-dialog-map activeDialogName)) intValue))
    )  
    ""
  )
)
(define (handle-dialog-click name attr)
  (define optionname (resolve-option-name name attr))
  (if (and activeDialogName (not (equal? optionname "")))
    ((get-fn-for-dialog-option activeDialogName optionname))
  )
)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; POPOVER
;;;;;;;;;;;;;;;;;;;;;;;;;;
(define menuOptions (map car uilist))

(define (textvalue name content action)
  (list name
    (list 
      (list name "layer" "basicui")
      (list name "scale" "0.004 0.01 0.004")
      (list name "value" content)
      (list name "popaction" action)
    )
  )
)
(define (popoverAction action)
  (define mappedAction (assoc action nameAction))
  (define actionName (car mappedAction))
  (if mappedAction 
    (let ((openedDialog ((cadr mappedAction))))
      (if (equal? openedDialog #t)
        (set! activeDialogName actionName)
        (set! activeDialogName #f)
      )
    )
  )
)

[define popItemPrefix ")text_"]
(define popItemPrefixLength (string-length popItemPrefix))

(define (popover-options elementName menuOptions)
  (define nextName (genNextName popItemPrefix))
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

(define sceneId #f);,
(define currOption #f)
(define (maybe-unload-popover) 
  (if sceneId (unload-scene sceneId))
  (set! sceneId #f)
  (set! currOption #f)
)
(define (change-popover elementName uiOption)
  (define isAlreadyLoaded (equal? currOption uiOption))
  (format #t "POPOVER: is already loaded: ~a\n" isAlreadyLoaded)
  (maybe-unload-popover)
  (if (not isAlreadyLoaded)
    (begin
      (set! sceneId (load-scene "./res/scenes/editor/popover.rawscene" (popover-options elementName (cadr (assoc uiOption uilist)))))
      (set! currOption uiOption)
      (format #t "object id: ~a\n" (lsobj-name "(dialog" sceneId))
      (enforce-layout (gameobj-id (lsobj-name "(dialog" sceneId)))  ; wait....why need two passed?
      (enforce-layout (gameobj-id (lsobj-name "(dialog" sceneId)))
    )
  )
)


(define (isPopoverElement name) (if (< (string-length name) popItemPrefixLength) #f (equal? (substring name 0 popItemPrefixLength) ")text_")) )

(define (fullElementName localname) (string-append "." (number->string mainSceneId) "/" localname))

(define (onObjUnselected) (maybe-unload-popover))
(define (onObjSelected gameobj color)
  (define objattrs (gameobj-attr gameobj))
  (define popoptionPair (assoc "popoption" objattrs))
  (define popactionPair (assoc "popaction" objattrs))
  (define popoption (if popoptionPair (cadr popoptionPair) ""))
  (define hasPopoption (not (equal? #f (member popoption menuOptions))))
  (define elementName (gameobj-name gameobj))
  (define ispopover (isPopoverElement elementName))

  (define dialogOptionPair (assoc "dialogoption" objattrs))
  (define dialogoption (if dialogOptionPair (cadr dialogOptionPair) ""))

  (format #t "popoption: ~a\n" popoption)
  (if hasPopoption
    (change-popover (fullElementName elementName) popoption)
    (maybe-unload-popover)
  )
  (if (and ispopover popactionPair) 
    (popoverAction (cadr popactionPair))
  )
  (if (not (or hasPopoption ispopover)) (maybe-unload-popover))
  (handle-dialog-click elementName objattrs)

  (if (not (equal? dialogoption ""))
    (cond
      ((equal? dialogoption "HIDE") (maybe-unload-sidepanel))
      (#t (change-sidepanel dialogoption (fullElementName "(menubar")))
    )
  )
)


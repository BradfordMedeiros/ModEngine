(define allowed-messages '(
    "diffuse_on" 
    "diffuse_off"
    "specular_on"
    "specular_off"
    "bloom_on"
    "bloom_off"
    "highlight_on"
    "highlight_off"
    "translate"
    "scale"
    "rotate"
  )
)

(define (makecamera) 
  (mk-obj-attr ">randomcamera" (list))
)
(define (makelight)
  (mk-obj-attr "!randomlight" (list))
)
(define (makeheightmap) 
  (mk-obj-attr "-randomhm" (list
    (list "map" "./res/heightmaps/dunes.jpg")
    (list "physics" "enabled")
    (list "dim" "10")
  ))
)

(define shouldBePainting #f)
(define (onMouse button action mods) 
  (if (and (equal? button 0) (equal? action 0))
    (set-state "paint_off")
  )
  (if (and (equal? button 0) (equal? action 1) shouldBePainting)
    (set-state "paint_on")
  )
)
(define (paint_on)  (set! shouldBePainting #t))
(define (paint_off) 
  (set! shouldBePainting #f)
  (set-state "paint_off")
)

(define selectedObject -1)
(define (next_texture)
  (set-state "next_texture")
  (set-istate "set_texture" (gameobj-id (lsobj-name "*activetexture")))

)
(define (prev_texture)
  (set-state "prev_texture")
  (set-istate "set_texture" (gameobj-id (lsobj-name "*activetexture")))
)
(define (set_texture)
  (if (not (equal? selectedObject -1))
    (set-istate "set_texture" selectedObject)
  )
)

(define fnMessages (list
  (list "makecamera" makecamera)
  (list "makelight"  makelight)
  (list "makeheightmap" makeheightmap)
  (list "paint_on"   paint_on)
  (list "paint_off"  paint_off)
  (list "nexttexture" next_texture)
  (list "prevtexture" prev_texture)
  (list "set_texture"  set_texture)
))

(define (setdrawopacity opacity)     (set-fstate "opacity"  opacity))
(define (setdrawsize        size)    (set-fstate "drawsize" size))
(define fnFloatMessages (list
  (list "drawopacity" setdrawopacity)
  (list "drawsize"    setdrawsize)
))


(define (onMessage message)
  (define fnToCall (assoc-ref fnMessages message))
  (if fnToCall
    ((car fnToCall))
    (if (member message allowed-messages)
      (set-state message)
    )
  )
)

(define (onFloatMessage message val)
  (define fnToCall (assoc-ref fnFloatMessages message))
  (if fnToCall
    ((car fnToCall) val)
  )
)

(define (onObjSelected obj color)
  (set! selectedObject (gameobj-id obj))
  (if (equal? (gameobj-id obj) (gameobj-id mainobj))
    (begin
      (set-fstate "drawcolor-r" (car   color))
      (set-fstate "drawcolor-g" (cadr  color))
      (set-fstate "drawcolor-b" (caddr color))
    )
  )
)

(define onModelViewer #t)
(define modelSelectorId (gameobj-id (lsobj-name "*modelviewer")))
;(define (onObjHover obj)
;  (if (equal? (gameobj-id obj) modelSelectorId)
;    (set! onModelViewer #t)
;  )
;)
;(define (onObjUnhover obj)
;  (if (equal? (gameobj-id obj) modelSelectorId)
;    (set! onModelViewer #f)
;  )
;)


(define allModels (ls-models))
(define modelIndex 0)
(define numModels  (length allModels))

(define (getmodels index)
  (define backlist (list-tail (ls-models) index))
  (if (<= (length backlist) 4)
    backlist
    (list-head backlist 4)
  )
)


(define (modelToTexture modelpath)
  (define elements (reverse (string-split modelpath #\/)))
  (define firstelement (car elements))
  (string-join (append (reverse (cdr elements)) (list (string-append firstelement "-tex.png"))) "/")
)
(define (listtextures index) (map modelToTexture (getmodels index)))

(define (setTexIfExists objname texture)
  (define objid (gameobj-id (lsobj-name objname)))
  (set-texture objid texture)
)

(define (setObjTextures textures index)
  (if (> (length textures) 0)
    (begin 
      (setTexIfExists (string-append "*pickmodel" (number->string index)) (car textures))
      (setObjTextures (cdr textures) (+ index 1))
    )
  )
)

(define (onScroll amount)
  (if onModelViewer
    (begin
      (set! modelIndex 
      (min numModels (max 0 (+ modelIndex (if (> amount 0) 1 -1)))))
      (setObjTextures (listtextures modelIndex) 1)
    )
  )
)

(onScroll 0)
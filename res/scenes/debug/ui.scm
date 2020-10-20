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
    "set_texture"
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
(define (next_texture)
  (set-state "next_texture")
)
(define (prev_texture)
  (set-state "prev_texture")
)

(define fnMessages (list
  (list "makecamera" makecamera)
  (list "makelight"  makelight)
  (list "makeheightmap" makeheightmap)
  (list "paint_on"   paint_on)
  (list "paint_off"  paint_off)
  (list "nexttexture" next_texture)
  (list "prevtexture" prev_texture)
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
  (if (equal? (gameobj-id obj) (gameobj-id mainobj))
    (begin
      (set-fstate "drawcolor-r" (car   color))
      (set-fstate "drawcolor-g" (cadr  color))
      (set-fstate "drawcolor-b" (caddr color))
    )
  )
)


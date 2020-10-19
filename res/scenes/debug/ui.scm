(define allowed-messages '(
    "diffuse_on" 
    "diffuse_off"
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
  (display "make camera placeholder\n")
)
(define (makelight)
  (display "make light placeholder\n")
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

(define fnMessages (list
  (list "makecamera" makecamera)
  (list "makelight"  makelight)
  (list "paint_on"   paint_on)
  (list "paint_off"  paint_off)
))

(define (setdrawopacity opacity) (set-fstate "opacity" opacity))
(define fnFloatMessages (list
  (list "drawopacity" setdrawopacity)
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


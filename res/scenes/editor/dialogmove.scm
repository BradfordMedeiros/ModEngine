(define objSelected #f)
(define hoveringObj #f)

(define (onMouse button action mods)
  (format #t "on mouse: ~a ~a ~a\n" button action mods)
  (if (and (equal? button 0) (equal? action 0))
    (begin
      (format #t "mouse up\n")
      (set! objSelected #f)
    )
  )
  (if (and (equal? button 0) (equal? action 1) hoveringObj)
    (begin
      (format #t "mouse down\n")
      (set! objSelected #t)
    )
  )
)

(define (onMouseMove xpos ypos ndcx ndcy)
  (define offset (list ndcx ndcy 0))
  (format #t "ndcx: ~a, ndcy: ~a\n" ndcx ndcy)
  (if objSelected 
    (begin
      (gameobj-setpos! mainobj offset)
      (enforce-layout (gameobj-id mainobj))
    )
  )
)

(define (onObjHover obj)
  (define id (gameobj-id obj))
  (if (equal? id (gameobj-id mainobj))
    (set! hoveringObj #t)
  )
)

(define (onObjUnhover obj)
  (define id (gameobj-id obj))
  (if (equal? id (gameobj-id mainobj))
    (set! hoveringObj #f)
  )
)

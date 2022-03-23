(define objSelected #f)
(define hoveringObj #f)

(define currNdi #f)
(define ndiMouseDown #f)
(define objposMouseDown #f)
; should calculate ndi when mouse down, and then on mouse move, this is delta 
; then add delta to gameobj-pos when the pos was down

(define (onMouse button action mods)
  (if (and (equal? button 0) (equal? action 0))
    (begin
      (set! objSelected #f)
      (set! ndiMouseDown #f)
      (set! objposMouseDown #f)
    )
  )
  (if (and (equal? button 0) (equal? action 1) hoveringObj)
    (begin
      (set! objSelected #t)
      (set! ndiMouseDown currNdi)
      (set! objposMouseDown (gameobj-pos mainobj))
    )
  )
)

(define (calcNdiOffset)
  (define diffX (- (car currNdi) (car ndiMouseDown)))
  (define diffY (- (cadr currNdi) (cadr ndiMouseDown)))
  (define newx (+ diffX (car objposMouseDown)))
  (define newy (+ diffY (cadr objposMouseDown)))
  (define oldz (caddr objposMouseDown))
  (list newx newy oldz)
)

(define (onMouseMove xpos ypos ndcx ndcy)
  (set! currNdi (list ndcx ndcy))
  ;(format #t "ndcx: ~a, ndcy: ~a\n" ndcx ndcy)
  (if objSelected 
    (begin
      (gameobj-setpos! mainobj (calcNdiOffset))
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

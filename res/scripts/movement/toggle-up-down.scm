(define doorDown #f)

(define (onKey key scancode action mods)
  (if (and (equal? key 75) (equal? action 0))
    (toggleDoor)
  )
)

(define (moveDoor delta)
  (let ((gameobj (lsobj-name "door")))
    (gameobj-setpos-rel! gameobj (list 0 delta 0))
  )
)

(define (toggleDoor)
  (set! doorDown (not doorDown))
  (moveDoor (if (equal? doorDown #f) -10 10))
)


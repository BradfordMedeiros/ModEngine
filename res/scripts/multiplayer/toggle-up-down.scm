(define doorDown #f)


(define (onMessage event )
  (display (string-append "event: " event "\n"))
  (if (equal? event "closedoor")
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


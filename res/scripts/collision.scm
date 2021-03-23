
(define last-ray-from-1 '(1 0 0))
(define last-ray-to-1 '(1 10 0))

(define (onCollideEnter obj1 obj2 hitpoint normal)
  (set! last-ray-from-1 hitpoint)
  (set! last-ray-to-1 (move-relative hitpoint normal 10))
  (display "obj1 id: ")
  (display (gameobj-name obj1))
  (display "\n")
  (display "obj2 id: ")
  (display (gameobj-name obj2))
  (display "\n")
)

(define (onCollideExit obj1 obj2)
  (display "on collide exit\n")
)

(define (onFrame)
  (draw-line last-ray-from-1 last-ray-to-1)
)



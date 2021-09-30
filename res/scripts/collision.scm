
(define last-ray-from-1 (list 1 0 0))
(define last-ray-to-1 (list 1 10 0))

(define (onGlobalCollideEnter obj1 obj2 hitpoint normal normal2)
  (display "on collide enter\n")
  (format #t "obj1 id: ~a, obj2 id: ~a, hitpoint: ~a, normal: ~a, normal2: ~a\n" (gameobj-name obj1) (gameobj-name obj2) hitpoint normal normal2)
  (set! last-ray-from-1 hitpoint)
  (set! last-ray-to-1 (move-relative hitpoint (orientation-from-pos (list 0 0 0) normal) 10))
)

(define (onGlobalCollideExit obj1 obj2)
  (display "on collide exit\n")
)

(define (onFrame)
  (draw-line last-ray-from-1 last-ray-to-1)
)



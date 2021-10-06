
(define velocity (list 
  (list 5 0 0)
  #f
  (list 5 5 0)
  #f
  (list -5 0 0)
  (list 0 -5 0)
))
(define listlength (length velocity))
(define (get-index) (modulo (inexact->exact (floor (time-seconds))) listlength))

(define (onFrame)
  (define newvelocity (list-ref velocity (get-index)))
  (format #t "velocity is: ~a\n" newvelocity)
  (if newvelocity
    (gameobj-setattr! mainobj (list (list "physics_velocity" newvelocity)))
  )
)
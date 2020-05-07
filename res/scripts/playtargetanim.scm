

(define (onMessage value)
  (define obj (lsobj-name "targetmodel"))
  (define animations (gameobj-animations obj))
  (gameobj-playanimation obj (car animations))
)
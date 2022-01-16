
(define (animation-name) (cadr (assoc "animation" (gameobj-attr mainobj))))
(define (onKey key scancode action mods)
  (if (equal? key 257) (gameobj-playanimation mainobj (animation-name)))
)


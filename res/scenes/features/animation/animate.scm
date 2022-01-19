
(define (animation-name) (cadr (assoc "animation" (gameobj-attr mainobj))))
(define (onKey key scancode action mods)
  (define animationname (animation-name))
  (if (equal? key 257) (gameobj-playanimation mainobj animationname))
  (format #t "playing animation ~a for model  name ~a" animationname (gameobj-name mainobj))
)


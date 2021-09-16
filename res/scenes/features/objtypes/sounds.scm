

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
    (playclip "&sample")
  )
)


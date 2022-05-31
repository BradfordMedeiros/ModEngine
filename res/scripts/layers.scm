
(define uivisible #f)

(define (toggle-ui-layer)
  (set! uivisible (not uivisible))
  (set-layer
    (list
      (list "basicui" "visible" (if uivisible "true" "false"))
    )
  )
)

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))      ; enter key
    (toggle-ui-layer)
  )
)


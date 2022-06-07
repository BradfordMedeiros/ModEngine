
(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 1 0))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  		)
	)
    (format #t "value is: ~a\n" (lsobj-name "someobj/Plane"))
    (gameobj-setattr! (lsobj-name "someobj/Plane") 
    	(list
    		(list "texture" "graphs-testplot")
    	)
    )
)



(define xRangeSecs 100)
(define yRangeFPS 500)
(define (convertX value) (- (* 2 (/ value xRangeSecs)) 1))
(define (convertY value) (- (* 2 (/ value yRangeFPS )) 1))


(define pointX #f)
(define pointY #f)
(define (plotPoint x y)
	(if (and pointX pointY)
		(draw-line (list pointX pointY 0) (list x y 0) #t textureId)
	)
	(set! pointX x)
	(set! pointY y)
)

(define minDrawingTime 0.1)
(define lastTime (time-seconds))
(define (onFrame)
	(define now (time-seconds))
	(define diff (- now lastTime))
	(if (> diff minDrawingTime)
		(begin
			(set! lastTime now)
      (let* ((currSec (time-seconds)) (fps (runstat "fps")) (coordX (convertX currSec)) (coordY (convertY fps)))
				;(format #t "(time = ~a[~a] , fps = ~a[~a])" currSec  coordX fps coordY)
				(plotPoint coordX coordY)
			)
		)
	)
)

(define textureId #f)
(define texturename "graphs-testplot")
(set! textureId (create-texture texturename 100 100))

(define (onKeyChar codepoint)
	(if (equal? codepoint 46) (free-texture texturename))           ; . 
	(if (equal? codepoint 47) (create-obj))                         ; / 
)


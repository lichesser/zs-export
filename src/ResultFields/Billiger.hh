<?hh //strict
namespace ElasticExport\ResultFields;

use Plenty\Modules\DataExchange\Contracts\ResultFields;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use Plenty\Modules\Helper\Services\ArrayHelper;

/**
 * Class Billiger
 * @package ElasticExport\ResultFields
 */
class Billiger extends ResultFields
{
    /*
	 * @var ArrayHelper
	 */
	private ArrayHelper $arrayHelper;

    /**
     * Billiger constructor.
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ArrayHelper $arrayHelper)
    {
		$this->arrayHelper = $arrayHelper;
    }

    /**
     * Generate result fields.
     * @param  array<FormatSetting> $formatSettings = []
     * @return array
     */
    public function generateResultFields(array<FormatSetting> $formatSettings = []):array<string, mixed>
    {
        $settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

        return [
            'itemBase'=> [
                'id',
                'producer',
            ],

            'itemDescription' => [
                'params' => [
                    'language' => $settings->get('lang'),
                ],
                'fields' => [
                    'name1',
                    'description',
                    'shortDescription',
                    'technicalData',
                ],
            ],

            'variationImageList' => [
                'params' => [
                        'type' => 'variation',
                        'referenceMarketplace' => $settings->get('reffererId'),
                ],
                'fields' => [
                    'imageId',
                    'type',
                    'fileType',
                    'path',
                    'position',
                    'cleanImageName',
                ]

            ],

            'variationBase' => [
                'availability',
                'model',
            ],

            'variationRetailPrice' => [
                'price',
            ],

            'variationStandardCategory' => [
                'params' => [
                    'plentyId' => $settings->get('referrerId'),
                ],
                'fields' => [
                    'categoryId'
                ],
            ],

            'variationBarcode' => [
                'params' => [
                    'EAN' => [
                        'barcodeType' => 'EANGeneral',
                    ],
                ],
                'fields' => [
                    'code',
                    'barcodeId',
                ]
            ],
        ];
    }
}
